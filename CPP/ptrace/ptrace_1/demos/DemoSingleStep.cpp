#include "DemoSingleStep.h"
#include "core/Debugger.h"
#include <cstdio>

/**
 * PT_STEP 原理：
 *   ARM64 通过设置 MDSCR_EL1.SS（Software Step 位）实现单步。
 *   每执行一条指令后 CPU 触发 debug exception，内核将其转化为 SIGTRAP。
 *   父进程的 waitpid 返回，可再次读取寄存器。
 *
 * ARM64 指令定长 4 字节，顺序执行时 PC 每步递增 4（跳转指令除外）。
 *
 * 本 demo 从子进程被 SIGSTOP 的位置开始单步，打印每步的 PC 值。
 * 初始几步处于 raise() 的返回路径中（libc 内部），
 * 之后进入 child_fn 的代码（volatile 赋值语句）。
 */
void DemoSingleStep::run() {
    auto dbg = Debugger::spawnChild([]() {
        // 子进程被放行后执行：几条可观察的赋值指令
        volatile int a = 10;
        volatile int b = 20;
        volatile int c = a + b;
        (void)c;
    });

    printf("  子进程 PID=%d，初始停止位置：\n", dbg->pid());
    printf("  PC = 0x%016llx\n\n", dbg->registersOf(0).pc());

    constexpr int kSteps = 12;
    printf("  开始单步执行 %d 步：\n", kSteps);

    for (int i = 0; i < kSteps; ++i) {
        auto ev = dbg->singleStep();

        if (ev.isStopped()) {
            printf("  step %2d: PC = 0x%016llx  signal=%d\n",
                   i + 1,
                   dbg->registersOf(0).pc(),
                   ev.signal);
        } else if (ev.isExited()) {
            printf("  step %2d: 进程已退出 (exitCode=%d)\n", i + 1, ev.exitCode);
            return;
        }
    }

    printf("\n  单步完成，放行子进程...\n");
    auto ev = dbg->continueUntilStop();
    if (ev.isExited())
        printf("  子进程退出，exitCode=%d\n", ev.exitCode);
}
