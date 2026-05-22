#include "MemoryAccess.h"
#include <stdexcept>
#include <string>

MemoryAccess::MemoryAccess(task_t task) : task_(task) {}

std::vector<uint8_t> MemoryAccess::read(mach_vm_address_t addr, size_t size) const {
    std::vector<uint8_t> buf(size);
    mach_vm_size_t outSize = 0;
    kern_return_t kr = mach_vm_read_overwrite(
        task_,
        addr,
        static_cast<mach_vm_size_t>(size),
        reinterpret_cast<mach_vm_address_t>(buf.data()),
        &outSize);
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("mach_vm_read_overwrite: ") + mach_error_string(kr));
    buf.resize(static_cast<size_t>(outSize));
    return buf;
}

void MemoryAccess::write(mach_vm_address_t addr, const void* data, size_t size) {
    // 数据段（rw-）：先确保有写权限，再写入
    kern_return_t kr = mach_vm_protect(
        task_, addr, size, FALSE, VM_PROT_READ | VM_PROT_WRITE);
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("mach_vm_protect(rw) failed: ") + mach_error_string(kr));

    kr = mach_vm_write(
        task_, addr,
        reinterpret_cast<vm_offset_t>(data),
        static_cast<mach_msg_type_number_t>(size));
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("mach_vm_write: ") + mach_error_string(kr));
}

void MemoryAccess::writeCode(mach_vm_address_t addr, const void* data, size_t size) {
    // 代码段（r-x）写入步骤：
    // 1. VM_PROT_COPY 触发 COW，将共享页变成私有可写页
    // 2. 写入新内容
    // 3. 恢复 r-x 权限，CPU 可执行
    kern_return_t kr = mach_vm_protect(
        task_, addr, size, FALSE,
        VM_PROT_READ | VM_PROT_WRITE | VM_PROT_COPY);
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("mach_vm_protect(rw+copy) failed: ") + mach_error_string(kr));

    kr = mach_vm_write(
        task_, addr,
        reinterpret_cast<vm_offset_t>(data),
        static_cast<mach_msg_type_number_t>(size));
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("mach_vm_write (code): ") + mach_error_string(kr));

    kr = mach_vm_protect(
        task_, addr, size, FALSE, VM_PROT_READ | VM_PROT_EXECUTE);
    if (kr != KERN_SUCCESS)
        throw std::runtime_error(
            std::string("mach_vm_protect(rx) failed: ") + mach_error_string(kr));
}

std::string MemoryAccess::readString(mach_vm_address_t addr, size_t maxLen) const {
    auto bytes = read(addr, maxLen);
    size_t len = 0;
    while (len < bytes.size() && bytes[len] != '\0') ++len;
    return std::string(bytes.begin(), bytes.begin() + static_cast<ptrdiff_t>(len));
}
