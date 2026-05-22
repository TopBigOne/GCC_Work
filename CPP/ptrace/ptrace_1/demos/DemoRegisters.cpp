#include "DemoRegisters.h"
#include "core/Debugger.h"
#include <cstdio>

/**
 * 演示流程：
 * 1. spawnChild() 内部：fork → 子进程执行 PT_TRACE_ME + raise(SIGSTOP)
 * 2. 父进程获得 Debugger 句柄，此时子进程已冻结
 * 3. 读取子进程第一个线程的 ARM64 寄存器
 * 4. 放行子进程（child_fn 为空 lambda，子进程直接退出）
 *
 * 子进程停止的位置：raise(SIGSTOP) 的系统调用返回处
 * → PC 指向 raise() 返回后的下一条指令
 */
void DemoRegisters::run() {
    auto dbg = Debugger::spawnChild([]() {
        // 子进程在父进程 continue 后会执行这里，然后退出
        // 故意放几条语句，便于单步 demo 观察
        volatile int x = 1;
        x = x + 1;
        (void)x;
    });

    printf("  子进程 PID = %d，已冻结，读取寄存器：\n\n", dbg->pid());
    dbg->registersOf(0).dump();

    printf("\n  放行子进程...\n");
    auto ev = dbg->continueUntilStop();
    if (ev.isExited())
        printf("  子进程以退出码 %d 结束\n", ev.exitCode);
}
