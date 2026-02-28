#pragma once
#include <iostream>
#include <memory>

/**
 * 懒汉式单例模式
 * 特点：延迟初始化，只有在第一次调用getInstance()时才创建实例
 * 优点：节省内存，按需创建
 * 缺点：线程不安全（基础版本）
 */
class LazySingleton {
private:
    static LazySingleton* instance;

    // 私有构造函数，防止外部创建实例
    LazySingleton() {
        std::cout << "LazySingleton 构造函数被调用" << std::endl;
    }

    // 禁用拷贝构造函数和赋值运算符
    LazySingleton(const LazySingleton&) = delete;
    LazySingleton& operator=(const LazySingleton&) = delete;

public:
    // 获取实例的静态方法
    static LazySingleton* getInstance() {
        if (instance == nullptr) {
            std::cout << "创建 LazySingleton 实例" << std::endl;
            instance = new LazySingleton();
        }
        return instance;
    }

    // 销毁实例（可选）
    static void destroyInstance() {
        if (instance) {
            delete instance;
            instance = nullptr;
            std::cout << "LazySingleton 实例已销毁" << std::endl;
        }
    }

    // 业务方法示例
    void doSomething() {
        std::cout << "LazySingleton 正在工作，地址: " << this << std::endl;
    }

    // 析构函数
    ~LazySingleton() {
        std::cout << "LazySingleton 析构函数被调用" << std::endl;
    }
};

// 静态成员变量定义
LazySingleton* LazySingleton::instance = nullptr;