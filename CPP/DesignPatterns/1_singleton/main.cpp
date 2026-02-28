#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

#include "lazy_singleton.h"
#include "eager_singleton.h"
#include "thread_safe_singleton.h"
#include "meyers_singleton.h"

void testLazySingleton() {
    std::cout << "\n=== 测试懒汉式单例 ===" << std::endl;

    // 第一次获取实例
    LazySingleton *instance1 = LazySingleton::getInstance();
    instance1->doSomething();

    // 第二次获取实例
    LazySingleton *instance2 = LazySingleton::getInstance();
    instance2->doSomething();

    // 验证是同一个实例
    std::cout << "实例1地址: " << instance1 << std::endl;
    std::cout << "实例2地址: " << instance2 << std::endl;
    std::cout << "是否为同一实例: " << (instance1 == instance2 ? "是" : "否") << std::endl;

    // 手动销毁
    LazySingleton::destroyInstance();
}

void testEagerSingleton() {
    std::cout << "\n=== 测试饿汉式单例 ===" << std::endl;

    // 饿汉式在程序启动时就已经创建了实例
    EagerSingleton &instance1 = EagerSingleton::getInstance();
    instance1.doSomething();

    EagerSingleton &instance2 = EagerSingleton::getInstance();
    instance2.doSomething();

    // 验证是同一个实例
    std::cout << "实例1地址: " << &instance1 << std::endl;
    std::cout << "实例2地址: " << &instance2 << std::endl;
    std::cout << "是否为同一实例: " << (&instance1 == &instance2 ? "是" : "否") << std::endl;
}

void threadSafeTest(int threadId) {
    ThreadSafeSingleton *instance = ThreadSafeSingleton::getInstance();
    std::cout << "线程 " << threadId << " 获取的实例地址: " << instance << std::endl;
    instance->doSomething();
}

void testThreadSafeSingleton() {
    std::cout << "\n=== 测试线程安全单例 ===" << std::endl;

    const int numThreads = 5;
    std::vector<std::thread> threads;

    // 创建多个线程同时获取实例
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(threadSafeTest, i);
    }

    // 等待所有线程完成
    for (auto &t: threads) {
        t.join();
    }

    // 手动销毁
    ThreadSafeSingleton::destroyInstance();
}

void meyersTest(int threadId) {
    MeyersSingleton &instance = MeyersSingleton::getInstance();
    std::cout << "线程 " << threadId << " 获取的Meyers实例地址: " << &instance << std::endl;
    instance.doSomething();
}

void testMeyersSingleton() {
    std::cout << "\n=== 测试Meyers单例 ===" << std::endl;

    // 单线程测试
    MeyersSingleton &instance1 = MeyersSingleton::getInstance();
    instance1.doSomething();

    MeyersSingleton &instance2 = MeyersSingleton::getInstance();
    instance2.doSomething();

    std::cout << "实例1地址: " << &instance1 << std::endl;
    std::cout << "实例2地址: " << &instance2 << std::endl;
    std::cout << "是否为同一实例: " << (&instance1 == &instance2 ? "是" : "否") << std::endl;

    // 多线程测试
    std::cout << "\n--- Meyers单例多线程测试 ---" << std::endl;
    const int numThreads = 3;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(meyersTest, i);
    }

    for (auto &t: threads) {
        t.join();
    }
}

void testLoggerSingleton() {
    std::cout << "\n=== 测试模板单例基类（Logger示例） ===" << std::endl;

    Logger &logger1 = Logger::getInstance();
    logger1.log("这是第一条日志消息");

    Logger &logger2 = Logger::getInstance();
    logger2.log("这是第二条日志消息");

    Logger &logger3 = Logger::getInstance();
    logger3.log("这是logger3的第一条日志消息");

    std::cout << "Logger1地址: " << &logger1 << std::endl;
    std::cout << "Logger2地址: " << &logger2 << std::endl;
    std::cout << "是否为同一实例: " << (&logger1 == &logger2 ? "是" : "否") << std::endl;
}

void performanceComparison() {
    std::cout << "\n=== 性能对比测试 ===" << std::endl;

    const int iterations = 1000000;

    // 测试Meyers单例性能
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        MeyersSingleton::getInstance();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto meyersDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // 测试线程安全单例性能
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        ThreadSafeSingleton::getInstance();
    }
    end = std::chrono::high_resolution_clock::now();
    auto threadSafeDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "Meyers单例 " << iterations << " 次调用耗时: " << meyersDuration.count() << " 微秒" << std::endl;
    std::cout << "线程安全单例 " << iterations << " 次调用耗时: " << threadSafeDuration.count() << " 微秒" << std::endl;

    // 清理
    ThreadSafeSingleton::destroyInstance();
}

int main() {
    std::cout << "=== C++单例模式学习示例 ===" << std::endl;
    std::cout << "程序启动..." << std::endl;

    // 注意：饿汉式单例在程序启动时就会创建实例

    // 测试各种单例实现
    testLazySingleton();
    testEagerSingleton();
    testThreadSafeSingleton();
    testMeyersSingleton();
    testLoggerSingleton();
    performanceComparison();

    std::cout << "\n=== 程序结束，观察析构函数调用顺序 ===" << std::endl;
    return 0;
}
