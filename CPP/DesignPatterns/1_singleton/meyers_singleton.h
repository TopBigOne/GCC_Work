#pragma once
#include <iostream>

/**
 * Meyers单例模式（推荐）, C++11后的最佳实践
 * 特点：利用C++11局部静态变量的特性
 * 优点：线程安全，延迟初始化，实现简洁，性能优秀
 * 原理：C++11保证局部静态变量的初始化是线程安全的
 */
class MeyersSingleton {
private:
    // 私有构造函数，防止外部创建实例
    MeyersSingleton() {
        std::cout << "MeyersSingleton 构造函数被调用" << std::endl;
    }

    // 禁用 拷贝构造函数 和 赋值运算符
    MeyersSingleton(const MeyersSingleton&) = delete;
    MeyersSingleton& operator=(const MeyersSingleton&) = delete;

public:
    // 获取实例的静态方法 - Meyers实现
    static MeyersSingleton& getInstance() {
        // 局部静态变量，C++11保证线程安全的初始化
        static MeyersSingleton instance;
        // 只在首次创建时打印，避免性能测试时输出过多
        static bool first_call = true;
        if (first_call) {
            std::cout << "获取 MeyersSingleton 实例" << std::endl;
            first_call = false;
        }
        return instance;
    }

    // 业务方法示例
    void doSomething() {
        std::cout << "MeyersSingleton 正在工作，地址: " << this << std::endl;
    }

    // 析构函数
    ~MeyersSingleton() {
        std::cout << "MeyersSingleton 析构函数被调用" << std::endl;
    }
};

/**
 * 模板版本的Meyers单例基类
 * 可以被继承来快速实现单例
 */
template<typename T>
class SingletonBase {
protected:
    SingletonBase() = default;
    ~SingletonBase() = default;

    // 禁用拷贝和赋值
    SingletonBase(const SingletonBase&) = delete;
    SingletonBase& operator=(const SingletonBase&) = delete;

public:
    static T& getInstance() {
        static T instance;
        return instance;
    }
};

// 使用模板基类的示例
class Logger : public SingletonBase<Logger> {
    friend class SingletonBase<Logger>; // 允许基类访问私有构造函数

private:
    Logger() {
        std::cout << "Logger 单例被创建" << std::endl;
    }

public:
    void log(const std::string& message) {
        std::cout << "[LOG] " << message << " (地址: " << this << ")" << std::endl;
    }

    ~Logger() {
        std::cout << "Logger 单例被销毁" << std::endl;
    }
};