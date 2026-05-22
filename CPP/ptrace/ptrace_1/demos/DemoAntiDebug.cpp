#include "DemoAntiDebug.h"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Part 1：PT_DENY_ATTACH 声明拒绝调试
 *
 * 子进程调用 PT_DENY_ATTACH，内核在进程描述符上设置 P_LNOATTACH 标志。
 * 之后任何进程（包括 lldb）调用 ptrace(PT_ATTACH) 都会收到 ENOTSUP。
 *
 * 注意：PT_DENY_ATTACH 与 PT_TRACE_ME 是正交的，互不干扰：
 *   P_LNOATTACH — 拒绝外部 PT_ATTACH
 *   P_LTRACED   — 允许父进程通过 PT_TRACE_ME 跟踪
 */
static void demoDenyAttach() {
    printf("  [Part 1] PT_DENY_ATTACH\n");

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程：声明拒绝被调试器附加
        int ret = ptrace(PT_DENY_ATTACH, 0, nullptr, 0);
        if (ret == -1) {
            // 仅当进程已被调试器跟踪（P_LTRACED 已置位）时才会失败
            // 此时内核直接调用 exit1() 终止进程，所以实际上执行不到这里
            printf("  [child] PT_DENY_ATTACH 失败（已被调试器跟踪，进程将被终止）\n");
        } else {
            printf("  [child] PT_DENY_ATTACH 成功，P_LNOATTACH 已设置\n");
            printf("  [child] 任何外部 ptrace(PT_ATTACH) 现在都会收到 ENOTSUP\n");
        }
        _exit(0);
    }

    waitpid(pid, nullptr, 0);
    printf("  [parent] 子进程已退出\n");
}

/**
 * Part 2：PT_TRACE_ME 检测已附加的调试器
 *
 * 若进程已被 lldb/gdb 附加（P_LTRACED 已置位），
 * ptrace(PT_TRACE_ME) 返回 -1 (EBUSY)。
 * 商业 App 常在启动早期用此方式检测调试器并做防护。
 */
static void detectDebugger() {
    printf("\n  [Part 2] PT_TRACE_ME 调试器检测\n");

    pid_t pid = fork();
    if (pid == 0) {
        int ret = ptrace(PT_TRACE_ME, 0, nullptr, 0);
        if (ret == -1) {
            printf("  [child] PT_TRACE_ME 返回 -1 (errno=%d: %s)\n",
                   errno, strerror(errno));
            printf("  [child] 结论：检测到调试器！\n");
        } else {
            printf("  [child] PT_TRACE_ME 返回 0，无调试器\n");
            raise(SIGSTOP);
        }
        _exit(0);
    }

    int status;
    waitpid(pid, &status, 0);
    if (WIFSTOPPED(status)) {
        printf("  [parent] 收到 SIGSTOP，放行子进程\n");
        ptrace(PT_CONTINUE, pid, reinterpret_cast<caddr_t>(1), 0);
        waitpid(pid, nullptr, 0);
    }
    printf("  [parent] 子进程已退出\n");
}

/**
 * Part 3：PT_DENY_ATTACH 在调试器下的致命行为（说明）
 */
static void explainFatalBehavior() {
    printf("\n  [Part 3] 调试器下调用 PT_DENY_ATTACH 的后果\n");
    printf("  若进程已被 lldb/gdb 附加（P_LTRACED 已置位），\n");
    printf("  调用 ptrace(PT_DENY_ATTACH) 触发内核 exit1()，\n");
    printf("  进程立即收到 SIGKILL，这就是商业 App 在 lldb 下直接闪退的原因。\n");
    printf("  绕过方式：在调用前用 lldb 断点跳过该 ptrace 调用，\n");
    printf("  或将二进制中的请求码 31 (PT_DENY_ATTACH) 替换为 0。\n");
}

void DemoAntiDebug::run() {
    demoDenyAttach();
    detectDebugger();
    explainFatalBehavior();
}
