#pragma once
#include "IDemo.h"

/**
 * DemoAntiDebug
 * 演示 macOS 反调试机制：
 *
 * Part 1 — PT_DENY_ATTACH：
 *   子进程调用 ptrace(PT_DENY_ATTACH)，标记自己不可被调试。
 *   父进程随后尝试 PT_ATTACH → 应收到 ENOTSUP 错误。
 *
 * Part 2 — PT_TRACE_ME 检测调试器：
 *   若进程已被 lldb/gdb 附加（P_LTRACED 标志置位），
 *   PT_TRACE_ME 返回 -1 (EBUSY)，可以此判断调试器存在。
 */
class DemoAntiDebug : public IDemo {
public:
    std::string name()        const override { return "AntiDebug"; }
    std::string description() const override {
        return "PT_DENY_ATTACH 阻止调试器附加 + PT_TRACE_ME 检测调试器";
    }
    void run() override;
};
