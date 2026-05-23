//
// Created by DEV on 2026/3/9.
//

#include "VariantDemo.h"

// index is :0 ,1 ,2,3
using MyVariant = std::variant<int, double, std::string, std::vector<int> >;

void VariantDemo::baseUsage() {
    MyVariant myVariant;
    myVariant = 24;
    std::cout << "current value:" << std::get<int>(myVariant) << std::endl;

    // 存储 double
    myVariant = 3.14;
    std::cout << "当前值: " << std::get<double>(myVariant) << "\n";

    // 存储 string
    myVariant = "Hello Variant";
    std::cout << "当前值: " << std::get<std::string>(myVariant) << "\n";

    // 存储 vector
    myVariant = std::vector{1, 2, 3};
    auto &vec = std::get<std::vector<int> >(myVariant);
    vec.push_back(4);
    std::cout << "Vector大小: " << vec.size() << "\n";

    // Note: 错误示例
    try {
        std::cout << "当前值: " << std::get<int>(myVariant) << "\n";
    } catch (const std::exception &e) {
        std::cerr << "exception: " << e.what() << "\n";
    }
}


void VariantDemo::typeChecking() {
    MyVariant myVariant;
    // myVariant = 3.14;
    myVariant = 3;
    // 检查当前类型索引
    std::cout << "类型索引 ：" << myVariant.index() << std::endl;
    // 类型检查
    if (std::holds_alternative<double>(myVariant)) {
        std::cout << "当前存储的是double" << std::endl;
    }

    // 安全访问-返回指针
    if (const auto *p = std::get_if<int>(&myVariant)) {
        std::cout << "Int 值:" << *p << std::endl;
    } else {
        std::cout << "当前值不是int 类型" << std::endl;
    }
}

void VariantDemo::withVisit() {
    MyVariant v1 = 42;
    MyVariant v2 = "Test";
    MyVariant v3 = std::vector{1, 2, 3};
    Visitor visitor;
    std::visit(visitor, v1);
    std::visit(visitor, v2);
    std::visit(visitor, v3);
}

void VariantDemo::withLambda() {
    MyVariant v = 3.14;
    std::visit([](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) {
            std::cout << "整数：" << arg << std::endl;
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << "双精度: " << arg * 2 << "\n";
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "字符串长度: " << arg.size() << "\n";
        } else {
            std::cout << "未知类型\n";
        }
    }, v);
}




