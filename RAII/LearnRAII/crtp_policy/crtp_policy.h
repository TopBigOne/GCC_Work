#pragma once
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>

// ============================================================
// 模块4：CRTP（奇异递归模板） + RAII
// 知识点：
//   1. CRTP 通过静态多态代替虚函数，零运行时开销
//   2. Policy 注入资源释放策略，不修改资源类本身
//   3. 对比：虚函数 vs CRTP 的优劣
// ============================================================

namespace raii {
namespace crtp {

// ----------------------------------------------------------
// 4.1 Policy 类（作为模板参数，必须完整定义在头文件）
// ----------------------------------------------------------
struct SilentPolicy {
    static void on_acquire(const std::string&) noexcept {}
    static void on_release(const std::string&) noexcept {}
};

struct LogPolicy {
    static void on_acquire(const std::string& name) {
        std::cout << "  [LogPolicy] 获取资源: " << name << "\n";
    }
    static void on_release(const std::string& name) {
        std::cout << "  [LogPolicy] 释放资源: " << name << "\n";
    }
};

struct CountingPolicy {
    static inline std::size_t acquire_count = 0;
    static inline std::size_t release_count = 0;

    static void on_acquire(const std::string& name) {
        std::cout << "  [CountingPolicy] 获取 #" << ++acquire_count
                  << ": " << name << "\n";
    }
    static void on_release(const std::string& name) {
        std::cout << "  [CountingPolicy] 释放 #" << ++release_count
                  << ": " << name << "\n";
    }
    static void report() {
        std::cout << "  [CountingPolicy] 统计 — 获取: " << acquire_count
                  << ", 释放: " << release_count << "\n";
    }
};

// ----------------------------------------------------------
// 4.2 CRTP 基类（模板，必须完整定义在头文件）
// ----------------------------------------------------------
template <typename Derived, typename Policy>
class ResourceBase {
public:
    explicit ResourceBase(std::string name) : name_(std::move(name)) {
        Policy::on_acquire(name_);
    }

    ~ResourceBase() {
        Policy::on_release(name_);
        static_cast<Derived&>(*this).do_release();
    }

    ResourceBase(const ResourceBase&)            = delete;
    ResourceBase& operator=(const ResourceBase&) = delete;

protected:
    const std::string& resource_name() const noexcept { return name_; }

private:
    std::string name_;
};

// ----------------------------------------------------------
// 4.3 具体资源类声明（非模板，实现在 crtp_policy.cpp）
// ----------------------------------------------------------
class LoggedBuffer : public ResourceBase<LoggedBuffer, LogPolicy> {
public:
    explicit LoggedBuffer(std::size_t size);
    void do_release();
    int& operator[](std::size_t i);

private:
    int*        data_;
    std::size_t size_;
};

class SilentFile : public ResourceBase<SilentFile, SilentPolicy> {
public:
    explicit SilentFile(const std::string& path);
    void do_release();
    void write(const char* s);

private:
    FILE*       file_;
    std::string path_;
};

class CountedSocket : public ResourceBase<CountedSocket, CountingPolicy> {
public:
    explicit CountedSocket(const std::string& endpoint);
    void do_release();
    void send(const std::string& data);

private:
    std::string endpoint_;
    bool        connected_;
};

void demo_crtp_policy();

}  // namespace crtp
}  // namespace raii
