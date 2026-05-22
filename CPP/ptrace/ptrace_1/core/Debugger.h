#pragma once
/**
 * Debugger.h
 * 设计模式：Facade + Factory Method
 *
 * Facade：将 ptrace、Mach API、寄存器访问、内存访问、断点管理
 *         统一封装在一个简洁的接口后面，隐藏底层复杂性。
 *
 * Factory Method：
 *   spawnChild(child_fn) — fork 子进程并自动建立调试关系
 *   attachTo(pid)        — 附加到已有进程
 */
#include "Breakpoint.h"
#include "MachTask.h"
#include "MemoryAccess.h"
#include "ProcessCtrl.h"
#include "RegisterAccess.h"
#include <functional>
#include <memory>
#include <vector>

class Debugger {
public:
    /**
     * Factory：fork 子进程，在其执行 child_fn 前暂停，
     * 父进程拿到 Debugger 句柄后可读写寄存器/内存。
     *
     * 子进程执行顺序：
     *   ptrace(PT_TRACE_ME) → raise(SIGSTOP) → child_fn() → exit(0)
     */
    static std::unique_ptr<Debugger> spawnChild(std::function<void()> child_fn);

    /**
     * Factory：附加到一个已经运行的进程（需要权限）。
     */
    static std::unique_ptr<Debugger> attachTo(pid_t pid);

    ~Debugger();

    // ----- 寄存器 -----
    RegisterAccess registersOf(size_t threadIdx = 0);

    // ----- 内存 -----
    MemoryAccess& memory() { return *memory_; }

    // ----- 执行控制 -----
    StopEvent continueUntilStop(int signal = 0);  // PT_CONTINUE + wait
    StopEvent singleStep(int signal = 0);         // PT_STEP + wait

    void detach();

    // ----- 断点（Command 模式） -----
    Breakpoint& addBreakpoint(mach_vm_address_t addr);
    void        removeBreakpoint(mach_vm_address_t addr);

    pid_t pid() const { return proc_->pid(); }

private:
    explicit Debugger(pid_t pid);

    void refreshThreads();
    void freeThreads();

    std::unique_ptr<MachTask>    task_;
    std::unique_ptr<MemoryAccess> memory_;
    std::unique_ptr<ProcessCtrl>  proc_;
    std::vector<std::unique_ptr<Breakpoint>> breakpoints_;

    thread_act_array_t       threads_     = nullptr;
    mach_msg_type_number_t   threadCount_ = 0;
};
