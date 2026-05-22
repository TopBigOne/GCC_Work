#include "RegisterAccess.h"
#include <stdexcept>
#include <string>
#include <cstdio>

RegisterAccess::RegisterAccess(thread_act_t thread) : thread_(thread) {}

arm_thread_state64_t RegisterAccess::read() const {
    arm_thread_state64_t state{};
    mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
    kern_return_t kr = thread_get_state(
        thread_,
        ARM_THREAD_STATE64,
        reinterpret_cast<thread_state_t>(&state),
        &count);
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("thread_get_state: ") + mach_error_string(kr));
    return state;
}

void RegisterAccess::write(const arm_thread_state64_t& state) {
    kern_return_t kr = thread_set_state(
        thread_,
        ARM_THREAD_STATE64,
        reinterpret_cast<thread_state_t>(const_cast<arm_thread_state64_t*>(&state)),
        ARM_THREAD_STATE64_COUNT);
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("thread_set_state: ") + mach_error_string(kr));
}

// ---- 快捷访问（必须使用 PAC 安全宏，不可直接读字段） ----

uint64_t RegisterAccess::pc() const {
    auto s = read();
    return static_cast<uint64_t>(arm_thread_state64_get_pc(s));
}

uint64_t RegisterAccess::sp() const {
    auto s = read();
    return static_cast<uint64_t>(arm_thread_state64_get_sp(s));
}

uint64_t RegisterAccess::fp() const {
    auto s = read();
    return static_cast<uint64_t>(arm_thread_state64_get_fp(s));
}

uint64_t RegisterAccess::lr() const {
    return read().__lr;
}

uint64_t RegisterAccess::x(size_t n) const {
    if (n > 28)
        throw std::out_of_range("x register index must be 0–28");
    return read().__x[n];
}

void RegisterAccess::dump() const {
    auto s = read();
    printf("  PC  = 0x%016llx  (程序计数器，等价 x86 RIP)\n",
           static_cast<uint64_t>(arm_thread_state64_get_pc(s)));
    printf("  SP  = 0x%016llx  (栈指针，等价 x86 RSP)\n",
           static_cast<uint64_t>(arm_thread_state64_get_sp(s)));
    printf("  FP  = 0x%016llx  (帧指针 x29，等价 x86 RBP)\n",
           static_cast<uint64_t>(arm_thread_state64_get_fp(s)));
    printf("  LR  = 0x%016llx  (链接寄存器 x30，保存返回地址)\n",
           static_cast<uint64_t>(s.__lr));
    for (int i = 0; i < 8; ++i)
        printf("  x%-2d = 0x%016llx\n", i, static_cast<uint64_t>(s.__x[i]));
}
