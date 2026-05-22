#include "Breakpoint.h"
#include <cstring>

Breakpoint::Breakpoint(mach_vm_address_t addr, MemoryAccess& mem)
    : addr_(addr), memory_(mem) {}

void Breakpoint::enable() {
    if (enabled_) return;

    // 1. 保存目标地址处的原始 4 字节指令
    auto bytes = memory_.read(addr_, sizeof(uint32_t));
    std::memcpy(&originalInst_, bytes.data(), sizeof(uint32_t));

    // 2. 写入 ARM64 BRK #0（0xD4200000）触发 SIGTRAP
    //    使用 writeCode：r-x → COW rw → write → r-x
    uint32_t brk = kARM64BrkInst;
    memory_.writeCode(addr_, &brk, sizeof(brk));

    enabled_ = true;
}

void Breakpoint::disable() {
    if (!enabled_) return;

    // 还原原始指令，再恢复 r-x
    memory_.writeCode(addr_, &originalInst_, sizeof(originalInst_));
    enabled_ = false;
}
