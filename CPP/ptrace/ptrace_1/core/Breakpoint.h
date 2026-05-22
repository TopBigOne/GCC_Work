#pragma once
/**
 * Breakpoint.h
 * 设计模式：Command
 *   - enable()  = 向目标地址写入 ARM64 BRK #0 指令（安装断点）
 *   - disable() = 还原原始指令（移除断点）
 *
 * ARM64 BRK #0 机器码：0xD4200000（4 字节定长）
 * 触发后进程收到 SIGTRAP，PC 指向 BRK 指令本身。
 * 恢复时：还原原始指令后直接 PT_CONTINUE，PC 无需回退。
 */
#include "MemoryAccess.h"
#include <cstdint>
#include <mach/mach_vm.h>

// ARM64 BRK #0 指令编码
static constexpr uint32_t kARM64BrkInst = 0xD4200000u;

class Breakpoint {
public:
    Breakpoint(mach_vm_address_t addr, MemoryAccess& mem);

    void enable();                              // 安装断点
    void disable();                             // 移除断点
    bool isEnabled() const { return enabled_; }
    mach_vm_address_t address() const { return addr_; }

private:
    mach_vm_address_t addr_;
    uint32_t          originalInst_{0};         // 保存被覆盖的原始指令
    bool              enabled_{false};
    MemoryAccess&     memory_;
};
