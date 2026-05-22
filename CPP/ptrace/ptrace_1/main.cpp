/**
 * main.cpp — ptrace demos 入口
 *
 * 设计模式总览：
 *   Facade       — Debugger 类：统一封装 ptrace + Mach API 的复杂性
 *   RAII         — MachTask：自动管理 Mach port 生命周期
 *   Command      — Breakpoint：enable()/disable() 封装断点安装/移除
 *   Factory      — Debugger::spawnChild() / Debugger::attachTo()
 *   Template     — IDemo：统一 demo 接口，main 通过多态批量运行
 *
 * 文件结构：
 *   core/
 *     MachTask.h          RAII Mach task wrapper
 *     RegisterAccess.h/cpp  ARM64 寄存器读写
 *     MemoryAccess.h/cpp    进程内存读写
 *     ProcessCtrl.h/cpp     ptrace 控制原语
 *     Breakpoint.h/cpp      软件断点 (Command)
 *     Debugger.h/cpp        调试器门面 (Facade + Factory)
 *   demos/
 *     IDemo.h               demo 接口 (Template Method)
 *     DemoTraceMe           PT_TRACE_ME 调试器检测
 *     DemoRegisters         读取子进程 ARM64 寄存器
 *     DemoMemory            读写子进程内存
 *     DemoSingleStep        PT_STEP 单步执行
 *     DemoAntiDebug         PT_DENY_ATTACH 反调试
 */
#include "demos/DemoAntiDebug.h"
#include "demos/DemoMemory.h"
#include "demos/DemoRegisters.h"
#include "demos/DemoSingleStep.h"
#include "demos/DemoTraceMe.h"
#include "demos/IDemo.h"

#include <cstdio>
#include <memory>
#include <vector>

static void printSeparator(const IDemo& demo, int idx) {
    printf("\n");
    printf("══════════════════════════════════════════════════\n");
    printf("  Demo %d: %s\n", idx, demo.name().c_str());
    printf("  %s\n", demo.description().c_str());
    printf("──────────────────────────────────────────────────\n");
}

int main() {
    std::vector<std::unique_ptr<IDemo>> demos;
    demos.push_back(std::make_unique<DemoTraceMe>());
    demos.push_back(std::make_unique<DemoRegisters>());
    demos.push_back(std::make_unique<DemoMemory>());
    demos.push_back(std::make_unique<DemoSingleStep>());
    demos.push_back(std::make_unique<DemoAntiDebug>());

    printf("ptrace demos — macOS ARM64\n");

    int idx = 1;
    for (auto& demo : demos) {
        printSeparator(*demo, idx++);
        try {
            demo->run();
        } catch (const std::exception& e) {
            printf("  [ERROR] %s\n", e.what());
        }
    }

    printf("\n══════════════════════════════════════════════════\n");
    printf("  全部 demo 运行完毕\n");
    printf("══════════════════════════════════════════════════\n\n");
    return 0;
}
