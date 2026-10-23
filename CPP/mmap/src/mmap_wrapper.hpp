#pragma once

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace mmap_learn {

// ============================================================
// RAII: 文件描述符自动管理
// ============================================================
class FileDescriptor {
    int fd_ = -1;

public:
    FileDescriptor() = default;

    explicit FileDescriptor(const char *path, int flags, mode_t mode = 0644)
        : fd_(::open(path, flags, mode)) {
        if (fd_ < 0) {
            throw std::runtime_error(std::string("open failed: ") + path + " - " + strerror(errno));
        }
    }

    ~FileDescriptor() {
        if (fd_ >= 0) ::close(fd_);
    }

    // 禁止拷贝
    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor &operator=(const FileDescriptor &) = delete;

    // 允许移动
    FileDescriptor(FileDescriptor &&other) noexcept : fd_(other.fd_) { other.fd_ = -1; }
    FileDescriptor &operator=(FileDescriptor &&other) noexcept {
        if (this != &other) {
            if (fd_ >= 0) ::close(fd_);
            fd_ = other.fd_;
            other.fd_ = -1;
        }
        return *this;
    }

    int get() const { return fd_; }
    bool valid() const { return fd_ >= 0; }

    off_t size() const {
        struct stat st {};
        if (fstat(fd_, &st) < 0) {
            throw std::runtime_error(std::string("fstat failed: ") + strerror(errno));
        }
        return st.st_size;
    }

    void truncate(off_t length) const {
        if (ftruncate(fd_, length) < 0) {
            throw std::runtime_error(std::string("ftruncate failed: ") + strerror(errno));
        }
    }
};

// ============================================================
// RAII: mmap 区域自动管理
// ============================================================
class MmapRegion {
    void *addr_ = MAP_FAILED;
    size_t length_ = 0;

public:
    MmapRegion() = default;

    MmapRegion(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
        : length_(length) {
        addr_ = ::mmap(addr, length, prot, flags, fd, offset);
        if (addr_ == MAP_FAILED) {
            throw std::runtime_error(std::string("mmap failed: ") + strerror(errno));
        }
    }

    ~MmapRegion() {
        if (addr_ != MAP_FAILED) {
            ::munmap(addr_, length_);
        }
    }

    // 禁止拷贝
    MmapRegion(const MmapRegion &) = delete;
    MmapRegion &operator=(const MmapRegion &) = delete;

    // 允许移动
    MmapRegion(MmapRegion &&other) noexcept : addr_(other.addr_), length_(other.length_) {
        other.addr_ = MAP_FAILED;
        other.length_ = 0;
    }

    MmapRegion &operator=(MmapRegion &&other) noexcept {
        if (this != &other) {
            if (addr_ != MAP_FAILED) ::munmap(addr_, length_);
            addr_ = other.addr_;
            length_ = other.length_;
            other.addr_ = MAP_FAILED;
            other.length_ = 0;
        }
        return *this;
    }

    void *data() { return addr_; }
    const void *data() const { return addr_; }
    size_t size() const { return length_; }
    bool valid() const { return addr_ != MAP_FAILED; }

    template<typename T>
    T *as() { return static_cast<T *>(addr_); }

    template<typename T>
    const T *as() const { return static_cast<const T *>(addr_); }

    // msync 刷盘
    int sync(int flags = MS_SYNC) {
        return ::msync(addr_, length_, flags);
    }

    // mprotect 修改权限
    int protect(int prot) {
        return ::mprotect(addr_, length_, prot);
    }

    // madvise 访问提示
    int advise(int advice) {
        return ::madvise(addr_, length_, advice);
    }

    // mlock 锁定内存
    int lock() { return ::mlock(addr_, length_); }
    int unlock() { return ::munlock(addr_, length_); }
};

// ============================================================
// Builder 模式: 链式配置 mmap 参数
// ============================================================
class MmapBuilder {
    void *addr_ = nullptr;
    size_t length_ = 0;
    int prot_ = PROT_READ;
    int flags_ = MAP_PRIVATE;
    int fd_ = -1;
    off_t offset_ = 0;

public:
    MmapBuilder &address(void *a) { addr_ = a; return *this; }
    MmapBuilder &length(size_t l) { length_ = l; return *this; }
    MmapBuilder &protection(int p) { prot_ = p; return *this; }
    MmapBuilder &flags(int f) { flags_ = f; return *this; }
    MmapBuilder &file(int fd) { fd_ = fd; return *this; }
    MmapBuilder &offset(off_t o) { offset_ = o; return *this; }

    // 便捷方法
    MmapBuilder &readOnly() { prot_ = PROT_READ; return *this; }
    MmapBuilder &readWrite() { prot_ = PROT_READ | PROT_WRITE; return *this; }
    MmapBuilder &shared() { flags_ = MAP_SHARED; return *this; }
    MmapBuilder &priv() { flags_ = MAP_PRIVATE; return *this; }
    MmapBuilder &anonymous() { flags_ |= MAP_ANON; fd_ = -1; return *this; }

    MmapRegion build() {
        if (length_ == 0) {
            throw std::invalid_argument("MmapBuilder: length must be > 0");
        }
        return MmapRegion(addr_, length_, prot_, flags_, fd_, offset_);
    }
};

} // namespace mmap_learn
