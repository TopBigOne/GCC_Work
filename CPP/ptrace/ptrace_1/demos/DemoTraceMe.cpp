#include "DemoTraceMe.h"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * PT_TRACE_ME 调试器检测原理：
 *
 * 内核为每个进程维护 P_LTRACED 标志。
 * - ptrace(PT_TRACE_ME) 成功：P_LTRACED 被设置，返回 0
 * - ptrace(PT_TRACE_ME) 失败：P_LTRACED 已经存在（已被调试），返回 -1 (EBUSY)
 *
 * 商业应用常在启动早期调用 PT_TRACE_ME，若失败则退出或混淆行为。
 */
void DemoTraceMe::run() {
    pid_t pid = fork();

    if (pid == 0) {
        // ---- 子进程：检测自己是否处于调试器中 ----
        int ret = ptrace(PT_TRACE_ME, 0, nullptr, 0);
        if (ret == -1) {
            printf("  [child] PT_TRACE_ME 返回 -1 (errno=%d: %s)\n",
                   errno, strerror(errno));
            printf("  [child] 结论：检测到调试器附加！\n");
        } else {
            printf("  [child] PT_TRACE_ME 返回 0，未检测到调试器\n");
            // 主动暂停，让父进程可以用 Mach API 观察（可选）
            raise(SIGSTOP);
            printf("  [child] 被父进程放行，继续运行\n");
        }
        _exit(0);
    }

    // ---- 父进程：等待子进程 ----
    int status = 0;
    waitpid(pid, &status, 0);

    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSTOP) {
        printf("  [parent] 子进程因 SIGSTOP 暂停，正在放行...\n");
        ptrace(PT_CONTINUE, pid, reinterpret_cast<caddr_t>(1), 0);
        waitpid(pid, nullptr, 0);
    }

    printf("  [parent] 子进程已退出\n");
    printf("  提示：用 lldb 运行本程序可触发\"检测到调试器\"分支\n");
}
