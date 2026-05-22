#include "Debugger.h"
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

// ---- Factory Methods ----

std::unique_ptr<Debugger> Debugger::spawnChild(std::function<void()> child_fn) {
    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error(std::string("fork: ") + strerror(errno));

    if (pid == 0) {
        // ---- 子进程 ----
        // 向内核声明允许父进程调试本进程
        if (ptrace(PT_TRACE_ME, 0, nullptr, 0) == -1) {
            perror("ptrace PT_TRACE_ME");
            _exit(1);
        }
        // 主动停下，通知父进程"我准备好了"
        raise(SIGSTOP);
        // 父进程 continueUntilStop() 后，子进程从这里继续
        child_fn();
        _exit(0);
    }

    // ---- 父进程：等待子进程停下 ----
    int status = 0;
    if (waitpid(pid, &status, 0) == -1)
        throw std::runtime_error(std::string("waitpid after fork: ") + strerror(errno));
    if (!WIFSTOPPED(status))
        throw std::runtime_error("child did not stop as expected after SIGSTOP");

    return std::unique_ptr<Debugger>(new Debugger(pid));
}

std::unique_ptr<Debugger> Debugger::attachTo(pid_t pid) {
    if (ptrace(PT_ATTACH, pid, nullptr, 0) == -1)
        throw std::runtime_error(std::string("PT_ATTACH: ") + strerror(errno));

    int status = 0;
    waitpid(pid, &status, 0);
    return std::unique_ptr<Debugger>(new Debugger(pid));
}

// ---- 私有构造 ----

Debugger::Debugger(pid_t pid)
    : task_(std::make_unique<MachTask>(pid)),
      memory_(std::make_unique<MemoryAccess>(task_->get())),
      proc_(std::make_unique<ProcessCtrl>(pid)) {
    refreshThreads();
}

Debugger::~Debugger() {
    freeThreads();
}

// ---- 线程列表管理 ----

void Debugger::refreshThreads() {
    freeThreads();
    kern_return_t kr = task_threads(task_->get(), &threads_, &threadCount_);
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("task_threads: ") + mach_error_string(kr));
}

void Debugger::freeThreads() {
    if (threads_) {
        vm_deallocate(mach_task_self(),
                      reinterpret_cast<vm_address_t>(threads_),
                      threadCount_ * sizeof(thread_act_t));
        threads_     = nullptr;
        threadCount_ = 0;
    }
}

// ---- 寄存器 ----

RegisterAccess Debugger::registersOf(size_t idx) {
    if (idx >= threadCount_)
        throw std::out_of_range("thread index out of range");
    return RegisterAccess(threads_[idx]);
}

// ---- 执行控制 ----

StopEvent Debugger::continueUntilStop(int signal) {
    proc_->continueExecution(signal);
    auto ev = proc_->wait();
    if (ev.isStopped()) refreshThreads();
    return ev;
}

StopEvent Debugger::singleStep(int signal) {
    proc_->singleStep(signal);
    auto ev = proc_->wait();
    if (ev.isStopped()) refreshThreads();
    return ev;
}

void Debugger::detach() {
    proc_->detach();
}

// ---- 断点 ----

Breakpoint& Debugger::addBreakpoint(mach_vm_address_t addr) {
    breakpoints_.push_back(std::make_unique<Breakpoint>(addr, *memory_));
    breakpoints_.back()->enable();
    return *breakpoints_.back();
}

void Debugger::removeBreakpoint(mach_vm_address_t addr) {
    for (auto it = breakpoints_.begin(); it != breakpoints_.end(); ++it) {
        if ((*it)->address() == addr) {
            (*it)->disable();
            breakpoints_.erase(it);
            return;
        }
    }
}
