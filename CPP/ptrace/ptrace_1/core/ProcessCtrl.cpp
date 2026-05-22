#include "ProcessCtrl.h"
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

ProcessCtrl::ProcessCtrl(pid_t pid) : pid_(pid) {}

void ProcessCtrl::continueExecution(int signal) {
    if (ptrace(PT_CONTINUE, pid_,
               reinterpret_cast<caddr_t>(1), signal) == -1)
        throw std::runtime_error(
            std::string("PT_CONTINUE: ") + strerror(errno));
}

void ProcessCtrl::singleStep(int signal) {
    if (ptrace(PT_STEP, pid_,
               reinterpret_cast<caddr_t>(1), signal) == -1)
        throw std::runtime_error(
            std::string("PT_STEP: ") + strerror(errno));
}

void ProcessCtrl::kill() {
    if (ptrace(PT_KILL, pid_, nullptr, 0) == -1)
        throw std::runtime_error(
            std::string("PT_KILL: ") + strerror(errno));
}

void ProcessCtrl::detach(int signal) {
    if (ptrace(PT_DETACH, pid_,
               reinterpret_cast<caddr_t>(1), signal) == -1)
        throw std::runtime_error(
            std::string("PT_DETACH: ") + strerror(errno));
}

StopEvent ProcessCtrl::wait() {
    int status = 0;
    if (waitpid(pid_, &status, 0) == -1)
        throw std::runtime_error(
            std::string("waitpid: ") + strerror(errno));

    StopEvent ev;
    if (WIFEXITED(status)) {
        ev.type     = StopEvent::Type::Exited;
        ev.exitCode = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        ev.type   = StopEvent::Type::Killed;
        ev.signal = WTERMSIG(status);
    } else if (WIFSTOPPED(status)) {
        ev.type   = StopEvent::Type::Stopped;
        ev.signal = WSTOPSIG(status);
    }
    return ev;
}
