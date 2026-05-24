#pragma once
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// ============================================================
// 模块3：代理模式 + RAII
// 知识点：
//   1. 透明代理：在不改变资源接口的情况下增加行为
//   2. 日志代理：记录每次资源访问
//   3. 懒加载代理：延迟初始化，首次使用时才构造资源
//   4. 访问控制代理：权限检查
// ============================================================

namespace raii {
namespace proxy {

// ----------------------------------------------------------
// 3.1 被代理的资源（非模板，声明在此，实现在 .cpp）
// ----------------------------------------------------------
class DatabaseConnection {
public:
    explicit DatabaseConnection(const std::string& url);
    ~DatabaseConnection();
    DatabaseConnection(const DatabaseConnection&)            = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    void               query(const std::string& sql);
    const std::string& url() const;
private:
    std::string url_;
};

// ----------------------------------------------------------
// 3.2 模板代理类 — 必须完整定义在头文件
// ----------------------------------------------------------
template <typename T>
class LoggingProxy {
public:
    template <typename... Args>
    explicit LoggingProxy(Args&&... args)
        : resource_(std::make_unique<T>(std::forward<Args>(args)...)) {
        log("LoggingProxy 构造完成");
    }

    ~LoggingProxy() { log("LoggingProxy 析构，释放资源"); }

    LoggingProxy(const LoggingProxy&)            = delete;
    LoggingProxy& operator=(const LoggingProxy&) = delete;

    T* operator->() { log("访问资源"); return resource_.get(); }
    T& operator*()  { log("解引用资源"); return *resource_; }
    T* get() noexcept { return resource_.get(); }

private:
    void log(const std::string& msg) const {
        auto ts = std::chrono::system_clock::now().time_since_epoch().count();
        std::cout << "  [LogProxy|" << ts % 100000 << "] " << msg << "\n";
    }
    std::unique_ptr<T> resource_;
};

template <typename T>
class LazyProxy {
public:
    using Factory = std::function<std::unique_ptr<T>()>;

    explicit LazyProxy(Factory factory) : factory_(std::move(factory)) {
        std::cout << "  [LazyProxy] 创建代理（资源尚未构造）\n";
    }

    ~LazyProxy() {
        if (resource_) std::cout << "  [LazyProxy] 析构，释放延迟创建的资源\n";
        else           std::cout << "  [LazyProxy] 析构，资源从未使用，无需释放\n";
    }

    LazyProxy(const LazyProxy&)            = delete;
    LazyProxy& operator=(const LazyProxy&) = delete;

    T* get() {
        if (!resource_) {
            std::cout << "  [LazyProxy] 首次访问，现在构造资源...\n";
            resource_ = factory_();
        }
        return resource_.get();
    }

    T* operator->() { return get(); }
    T& operator*()  { return *get(); }
    bool is_initialized() const noexcept { return resource_ != nullptr; }

private:
    Factory            factory_;
    std::unique_ptr<T> resource_;
};

template <typename T>
class GuardedProxy {
public:
    using Predicate = std::function<bool()>;

    GuardedProxy(std::unique_ptr<T> resource, Predicate can_access)
        : resource_(std::move(resource)), can_access_(std::move(can_access)) {
        std::cout << "  [GuardedProxy] 代理创建，资源受保护\n";
    }

    ~GuardedProxy() { std::cout << "  [GuardedProxy] 析构，资源释放\n"; }

    GuardedProxy(const GuardedProxy&)            = delete;
    GuardedProxy& operator=(const GuardedProxy&) = delete;

    T& operator*()  { check_access(); return *resource_; }
    T* operator->() { check_access(); return resource_.get(); }

private:
    void check_access() {
        if (!can_access_()) throw std::runtime_error("GuardedProxy: 访问被拒绝");
        std::cout << "  [GuardedProxy] 权限验证通过\n";
    }
    std::unique_ptr<T> resource_;
    Predicate          can_access_;
};

void demo_proxy_resource();

}  // namespace proxy
}  // namespace raii
