#pragma once
#include "IDemo.h"

/**
 * DemoRegisters
 * 演示通过 Debugger Facade 读取子进程 ARM64 寄存器：
 *   fork → 子进程暂停 → 父进程读取 PC/SP/FP/x0~x7 → 放行子进程
 */
class DemoRegisters : public IDemo {
public:
    std::string name()        const override { return "ReadRegisters"; }
    std::string description() const override {
        return "暂停子进程后读取其 ARM64 寄存器（PC/SP/FP/x0~x7）";
    }
    void run() override;
};
