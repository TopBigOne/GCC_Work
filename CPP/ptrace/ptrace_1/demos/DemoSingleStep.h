#pragma once
#include "IDemo.h"

/**
 * DemoSingleStep
 * 演示 PT_STEP 单步执行：
 *   子进程暂停后，父进程每次调用 PT_STEP 让子进程
 *   执行恰好一条 ARM64 指令，然后再次停下。
 *   父进程在每步后打印 PC，可以观察 PC 按 4 字节递增。
 */
class DemoSingleStep : public IDemo {
public:
    std::string name()        const override { return "SingleStep"; }
    std::string description() const override {
        return "PT_STEP 逐条指令执行，每步打印 PC（ARM64 每条指令 4 字节）";
    }
    void run() override;
};
