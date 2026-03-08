//
// Created by DEV on 2026/3/7.
//

#include "CaptureList.h"

#include <iostream>

/**
 * 捕获列表的解决方案：显式声明要捕获的外部变量
 */
void CaptureList::test_one() {
    int count = 1;
    auto lambda = [count] {
        std::cout << "count: " << count << "\n";
    };

    // 调用 lambda 函数
    lambda();
}

/***
(1) 值捕获 (Capture by Value)
特点：创建外部变量的副本
语法：[var1, var2]
*/
void CaptureList::test_two() {
    int b = 1;
    // 捕获时复制 a 的值
    auto lambda = [b] {
        std::cout << "b in lambda :" << b << std::endl; // 输出 10（即使外部 a 改变）
    };
    b = 20;
    std::cout << "b in outer :" << b << std::endl; // 输出 20
    lambda();
}

/**
*  引用捕获 (Capture by Reference)
特点：直接操作原变量
语法：[&var1, &var2]
 */
void CaptureList::test_three() {
    int a = 20;
    auto lambda = [&a] {
        a = 22; //直接修改外部的变量
    };
    lambda();
    std::cout << "a: " << a << std::endl; // 2输出 22
}

void startAsyncTask(std::function<void()> cal) {
    // 假设在另外一个线程执行callback
}

/**
 * 关键作用：控制 Lambda 的闭包行为
 */
void CaptureList::test_Four() {
    std::cout << "  在异步回调中保留上下文" << std::endl;
    int retryCount = 3;
    startAsyncTask([retryCount] {
    });
}

void CaptureList::test_Five() {
    std::vector<int> data = {1, 2, 3, 4, 5};
    int threshold = 2;
    auto count = std::count_if(data.begin(), data.end(), [threshold](int x) {
        return x > threshold;
    });
    std::cout << "count : " << count << std::endl;
}

/**
 * 允许修改值捕获的变量（默认 Lambda 的 operator() 是 const）
 */
void CaptureList::test_Six() {
    std::cout << "   mutable 关键字" << std::endl;
    int x = 0;
    auto lambda = [x]()mutable {
        x++;
        std::cout << "x in lambda :" << x << std::endl;
    };

    std::cout << "x in outer  :" << x << std::endl;

    lambda();
    lambda();
    std::cout << "x in outer  :" << x << std::endl;
}


auto createLambda() {
    int localVar = 42;
    return [&localVar] {
        //  危险！localVar 即将销毁
        std::cout << "localVar in lambda  :" << localVar << std::endl;
    }; // NOTE: localVar 离开作用域被销毁
}

/**
 * 引用捕获可能导致悬垂引用
 */
void CaptureList::test_Seven() {
    std::cout << "   生命周期问题" << std::endl;
    auto lambda = createLambda();
    lambda();
}

auto makeCounter() {
    int counter = 0;
    return [counter]()mutable {
        return counter++;
    };
}

void CaptureList::test_eight() {
    auto counter = makeCounter();
    std::cout << "   计数器生成器" << std::endl;

    std::cout << counter() << std::endl; // 1
    std::cout << counter() << std::endl; // 2（每次调用保持独立状态）
}

struct Person {
    std::string name;
    int age;
};


/**
 * 打印人员列表的辅助函数
 * @param people
 */
void printPeople(const std::vector<Person>& people) {
    for (const auto& p : people) {
        std::cout << p.name << " (" << p.age << ")\n";
    }
}
void CaptureList::test_nine() {
    Person p1 = {.name = "Jim", .age = 2};
    Person p2 = {.name = "Tom", .age = 6};
    Person p3 = {.name = "White", .age = 8};
    Person p4 = {.name = "zhizi", .age = 4};

    std::vector<Person> people = {p1, p2, p3, p4};
    std::cout << "--- 排序前 ---\n";
    printPeople(people);
    int targetAge = 30;
    std::sort(people.begin(), people.end(),
              [targetAge](const Person &a, const Person &b) {
                  return abs(a.age - targetAge) < abs(b.age - targetAge);
              });

    std::cout << "\n--- 按距离 " << targetAge << " 岁的年龄差排序后 ---\n";
    printPeople(people);
}


