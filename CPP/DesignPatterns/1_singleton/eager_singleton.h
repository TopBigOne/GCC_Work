#pragma once
#include <iostream>

/**
 * 饿汉式单例模式
 * 特点：程序启动时就创建实例
 * 优点：线程安全，实现简单
 * 缺点：可能造成内存浪费（如果不使用的话）
 */
class EagerSingleton {
private:
    // 静态实例在程序启动时就创建
    static EagerSingleton instance;

    // 私有构造函数，防止外部创建实例
    EagerSingleton() {
        std::cout << "EagerSingleton 构造函数被调用（程序启动时）" << std::endl;
    }

    // 禁用拷贝构造函数和赋值运算符
    EagerSingleton(const EagerSingleton&) = delete;
    EagerSingleton& operator=(const EagerSingleton&) = delete;

public:
    // 获取实例的静态方法
    static EagerSingleton& getInstance() {
        std::cout << "获取 EagerSingleton 实例" << std::endl;
        return instance;
    }

    // 业务方法示例
    void doSomething() {
        std::cout << "EagerSingleton 正在工作，地址: " << this << std::endl;
    }

    // 析构函数
    ~EagerSingleton() {
        std::cout << "EagerSingleton 析构函数被调用" << std::endl;
    }
};

// 静态成员变量定义 - 在程序启动时就创建实例
EagerSingleton EagerSingleton::instance;