#pragma once
/**
 * ProcessCtrl.h
 * 封装 ptrace 控制原语（继续/单步/终止/分离）。
 *
 * StopEvent 描述 waitpid 返回后进程的停止原因。
 */
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

struct StopEvent {
    enum class Type { Stopped, Exited, Killed };

    Type type     = Type::Stopped;
    int  signal   = 0;    // 有效当 type == Stopped 或 Killed
    int  exitCode = 0;    // 有效当 type == Exited

    bool isStopped() const { return type == Type::Stopped; }
    bool isExited()  const { return type == Type::Exited;  }
};

class ProcessCtrl {
public:
    explicit ProcessCtrl(pid_t pid);

    void continueExecution(int signal = 0);   // PT_CONTINUE
    void singleStep(int signal = 0);          // PT_STEP
    void kill();                              // PT_KILL
    void detach(int signal = 0);             // PT_DETACH

    // 阻塞等待进程停止/退出，返回停止原因
    StopEvent wait();

    pid_t pid() const { return pid_; }

private:
    pid_t pid_;
};
