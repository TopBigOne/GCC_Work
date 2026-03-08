#include <iostream>
#include <functional>

#include "Calculator.h"
#include "EventSystem.h"
#include "StrategyPatternSort.h"


int add(int a, int b) {
    return a + b;
}

struct Multiply {
    int operator()(int a, int b) {
        return a * b;
    }
};


void processData(const std::vector<int> &data, std::function<void(int)> callback) {
    for (int num: data) {
        // (num * 2)作为 callback 的参数；
        callback(num * 2);
    }
}

void testBaseUsed();

void testMemberFunc();

void testStoreCollection();

void testCallback();

void testEventSystem();

void testStrategyPatternSort();


/**
 * 案例 1：基础用法
 */
void testBaseUsed() {
    puts("=============基本用法=============");
    // define function
    std::function<int(int, int)> func;

    // case 1: bind normal func
    func = add;
    std::cout << "Add     : " << func(3, 4) << std::endl;

    // case 2: bind 仿函数对象
    func = Multiply{};
    std::cout << "Multiply: " << func(6, 4) << std::endl;

    // case 3: bind lambda
    func = [](int a, int b) { return a - b; };
    std::cout << "Subtract: " << func(10, 5) << std::endl;
}


void testMemberFunc() {
    puts("=============成员函数=============");
    Calculator calc;
    // case 1: 解绑非静态成员函数（需要实例对象）
    std::function<int(int, int)> func1 = std::bind(
        &Calculator::divide,
        &calc,
        std::placeholders::_1,
        std::placeholders::_2
    );

    auto result = func1(3, 4);
    std::cout << "Divide: " << result << std::endl;

    // case 2: 绑定静态成员函数
    std::function<int(int, int)> func2 = &Calculator::mod;
    result = func2(10, 3);
    std::cout << "Mod   : " << result << std::endl;
}

void testStoreCollection() {
    puts("=============存储回调到容器=============");
    std::vector<std::function<void()> > tasks;

    tasks.push_back([] { std::cout << "Task 1\n"; });
    tasks.push_back([] { std::cout << "Task 2\n"; });
    int x = 5;
    tasks.push_back([x] {
        std::cout << "Task 3, x:" << x << std::endl;;
    });

    // execute all tasks
    for (auto singleTask: tasks) {
        singleTask();
    }
}


void testCallback() {
    puts("=============作为函数参数（回调机制）=============");
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    processData(numbers, [](int x) {
        std::cout << "Processed: " << x << std::endl;
    });
}

void testEventSystem() {
    puts("=============事件系统（观察者模式）=============");
    EventSystem eventSystem;
    // case 1: add one
    eventSystem.addListener([](const std::string &msg) {
        std::cout << "Listener 1: " << msg << std::endl;
    });

    //case 2: add two（带捕获）
    std::string prefix = "Log>>";
    eventSystem.addListener([prefix](const std::string &msg) {
        std::cout << "Listener 2: " << prefix << " " << msg << std::endl;
    });
    eventSystem.triggerEvent("Hello Event");
}

void testStrategyPatternSort() {
    StrategyPatternSort strategy;
    strategy.test();
}


int main() {
    testBaseUsed();
    testMemberFunc();
    testStoreCollection();
    testCallback();
    testEventSystem();
    testStrategyPatternSort();
    return 0;
}
