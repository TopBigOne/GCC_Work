#pragma once
#include "IDemo.h"

/**
 * DemoTraceMe
 * 演示 PT_TRACE_ME：
 *   子进程调用 ptrace(PT_TRACE_ME)，若已被调试器附加则返回 -1。
 *   这是在 macOS 上检测调试器存在与否最常见的方式。
 */
class DemoTraceMe : public IDemo {
public:
    std::string name()        const override { return "PT_TRACE_ME"; }
    std::string description() const override {
        return "通过 PT_TRACE_ME 返回值检测当前进程是否处于调试器中";
    }
    void run() override;
};
