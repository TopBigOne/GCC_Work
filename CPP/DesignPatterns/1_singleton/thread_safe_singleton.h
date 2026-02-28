#pragma once
#include <iostream>
#include <mutex>
#include <memory>

/**
 * 线程安全的单例模式
 * 特点：使用双重检查锁定（Double-Checked Locking）
 * 优点：线程安全，延迟初始化，性能较好
 * 缺点：实现复杂，需要考虑内存序列
 */
class ThreadSafeSingleton {
private:
    static std::atomic<ThreadSafeSingleton*> instance;
    static std::mutex mutex_;

    // 私有构造函数，防止外部创建实例
    ThreadSafeSingleton() {
        std::cout << "ThreadSafeSingleton 构造函数被调用" << std::endl;
    }

    // 禁用拷贝构造函数和赋值运算符
    ThreadSafeSingleton(const ThreadSafeSingleton&) = delete;
    ThreadSafeSingleton& operator=(const ThreadSafeSingleton&) = delete;

public:
    // 获取实例的静态方法 - 双重检查锁定
    static ThreadSafeSingleton* getInstance() {
        ThreadSafeSingleton* tmp = instance.load(std::memory_order_relaxed);
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(mutex_);
            tmp = instance.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                std::cout << "创建 ThreadSafeSingleton 实例（线程安全）" << std::endl;
                tmp = new ThreadSafeSingleton();
                instance.store(tmp, std::memory_order_relaxed);
            }
        }
        return tmp;
    }

    // 销毁实例（可选）
    static void destroyInstance() {
        std::lock_guard<std::mutex> lock(mutex_);
        ThreadSafeSingleton* tmp = instance.load(std::memory_order_relaxed);
        if (tmp) {
            delete tmp;
            instance.store(nullptr, std::memory_order_relaxed);
            std::cout << "ThreadSafeSingleton 实例已销毁" << std::endl;
        }
    }

    // 业务方法示例
    void doSomething() {
        std::cout << "ThreadSafeSingleton 正在工作，地址: " << this << std::endl;
    }

    // 析构函数
    ~ThreadSafeSingleton() {
        std::cout << "ThreadSafeSingleton 析构函数被调用" << std::endl;
    }
};

// 静态成员变量定义
std::atomic<ThreadSafeSingleton*> ThreadSafeSingleton::instance{nullptr};
std::mutex ThreadSafeSingleton::mutex_;