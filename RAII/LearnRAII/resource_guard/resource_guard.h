#pragma once
#include <cstdio>
#include <cstddef>
#include <iostream>
#include <string>
#include <typeinfo>

// ============================================================
// 模块1：RAII 基础
// 知识点：
//   1. 构造获取资源，析构释放资源
//   2. 禁用拷贝，实现移动语义（Rule of Five）
//   3. 与 std::unique_ptr 对比
// ============================================================

namespace raii {

// ----------------------------------------------------------
// 1.1 FileGuard — 管理 FILE* 生命周期（声明）
// ----------------------------------------------------------
class FileGuard {
public:
    explicit FileGuard(const std::string& path, const char* mode = "w+");
    ~FileGuard() noexcept;

    FileGuard(const FileGuard&)            = delete;
    FileGuard& operator=(const FileGuard&) = delete;

    FileGuard(FileGuard&& other) noexcept;
    FileGuard& operator=(FileGuard&& other) noexcept;

    FILE* get() const noexcept;
    bool  is_open() const noexcept;
    void  write(const std::string& text);

private:
    FILE* file_;
};

// ----------------------------------------------------------
// 1.2 MemoryGuard<T> — 管理堆数组生命周期
//     模板类必须完整定义在头文件中
// ----------------------------------------------------------
template <typename T>
class MemoryGuard {
public:
    explicit MemoryGuard(std::size_t size)
        : ptr_(new T[size]()), size_(size) {
        std::cout << "[MemoryGuard] 分配 " << size
                  << " 个 " << typeid(T).name() << "\n";
    }

    ~MemoryGuard() noexcept {
        delete[] ptr_;
        std::cout << "[MemoryGuard] 释放内存\n";
    }

    MemoryGuard(const MemoryGuard&)            = delete;
    MemoryGuard& operator=(const MemoryGuard&) = delete;

    MemoryGuard(MemoryGuard&& other) noexcept
        : ptr_(other.ptr_), size_(other.size_) {
        other.ptr_  = nullptr;
        other.size_ = 0;
    }

    T&          operator[](std::size_t i) { return ptr_[i]; }
    const T&    operator[](std::size_t i) const { return ptr_[i]; }
    T*          get() const noexcept { return ptr_; }
    std::size_t size() const noexcept { return size_; }

private:
    T*          ptr_;
    std::size_t size_;
};

void demo_resource_guard();

}  // namespace raii
