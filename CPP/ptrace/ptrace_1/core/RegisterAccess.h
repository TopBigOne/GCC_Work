#pragma once
/**
 * RegisterAccess.h
 * 封装 ARM64 寄存器的读写操作。
 *
 * 注意：Apple Silicon 启用了 PAC（指针认证），
 * 访问 pc/sp/fp 必须通过专用宏，而不能直接读取结构体字段。
 */
#include <cstdint>
#include <mach/arm/thread_status.h>
#include <mach/mach.h>

class RegisterAccess {
public:
    explicit RegisterAccess(thread_act_t thread);

    // 读取完整 ARM64 寄存器快照
    arm_thread_state64_t read() const;
    // 写回修改后的寄存器快照
    void write(const arm_thread_state64_t& state);

    // 常用寄存器快捷访问（内部调用 read()，适合单次访问）
    uint64_t pc() const;  // 程序计数器（等价于 x86 RIP）
    uint64_t sp() const;  // 栈指针（等价于 x86 RSP）
    uint64_t fp() const;  // 帧指针 x29（等价于 x86 RBP）
    uint64_t lr() const;  // 链接寄存器 x30（保存返回地址）
    uint64_t x(size_t n) const;  // 通用寄存器 x0~x28

    void dump() const;  // 打印所有常用寄存器

private:
    thread_act_t thread_;
};
