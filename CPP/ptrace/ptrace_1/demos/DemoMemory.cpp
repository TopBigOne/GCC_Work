#include "DemoMemory.h"
#include "core/Debugger.h"
#include <cstdio>
#include <cstring>
#include <mach/mach_vm.h>

/**
 * fork 后父子进程虚拟地址空间相同（COW），
 * 所以父进程可以直接用 g_buf 的地址来访问子进程的同名变量。
 *
 * 演示流程：
 * 1. g_buf 初始值 "hello, ptrace!"（fork 前确定）
 * 2. 子进程停下后，父进程用 mach_vm_read_overwrite 读取子进程 g_buf
 * 3. 父进程用 mach_vm_write 向子进程 g_buf 写入 "PATCHED!"
 * 4. 父进程放行子进程
 * 5. 子进程（child_fn）打印 g_buf，验证内容已被父进程修改
 */

// 文件域全局变量，fork 后父子进程虚拟地址相同
static volatile char g_buf[32] = "hello, ptrace!";

void DemoMemory::run() {
    // 在父进程打印原始值（此时还未 fork）
    printf("  [parent] fork 前 g_buf = \"%s\"  addr = %p\n",
           (const char*)g_buf, (void*)g_buf);

    auto dbg = Debugger::spawnChild([]() {
        // 子进程被放行后执行此处，此时 g_buf 已被父进程修改
        printf("  [child]  child_fn 读到 g_buf = \"%s\"\n",
               (const char*)g_buf);
    });

    // ---- 父进程：子进程已停下，g_buf 尚未被 child_fn 修改 ----

    auto addr = reinterpret_cast<mach_vm_address_t>(g_buf);

    // 1. 读取子进程内存
    std::string before = dbg->memory().readString(addr);
    printf("  [parent] 读取子进程 g_buf = \"%s\"\n", before.c_str());

    // 2. 写入新值
    const char newVal[] = "PATCHED!";
    dbg->memory().write(addr, newVal, sizeof(newVal));
    printf("  [parent] 写入子进程 g_buf = \"%s\"\n", newVal);

    // 3. 验证：再读一次确认写入成功
    std::string after = dbg->memory().readString(addr);
    printf("  [parent] 验证读回   g_buf = \"%s\"\n", after.c_str());

    // 4. 放行子进程，child_fn 将打印修改后的值
    printf("  [parent] 放行子进程...\n");
    auto ev = dbg->continueUntilStop();
    if (ev.isExited())
        printf("  [parent] 子进程退出，exitCode=%d\n", ev.exitCode);
}
