#pragma once
/**
 * MemoryAccess.h
 * 封装对目标进程虚拟内存的读写操作。
 *
 * write()     - 写入数据段（.data/.bss），直接修改读写页
 * writeCode() - 写入代码段（.text），先通过 COW 拷贝页，再写入，最后恢复 r-x
 */
#include <cstdint>
#include <cstring>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <string>
#include <vector>

class MemoryAccess {
public:
    explicit MemoryAccess(task_t task);

    // 读取任意内存
    std::vector<uint8_t> read(mach_vm_address_t addr, size_t size) const;

    // 写入数据内存（rw 页）
    void write(mach_vm_address_t addr, const void* data, size_t size);

    // 写入代码内存（r-x 页），内部处理 COW 和权限切换
    void writeCode(mach_vm_address_t addr, const void* data, size_t size);

    // 读取以 '\0' 结尾的字符串
    std::string readString(mach_vm_address_t addr, size_t maxLen = 256) const;

    // 泛型读取：将指定地址的内存解释为 T 类型
    template <typename T>
    T readAs(mach_vm_address_t addr) const {
        auto bytes = read(addr, sizeof(T));
        T value{};
        std::memcpy(&value, bytes.data(), sizeof(T));
        return value;
    }

private:
    task_t task_;
};
