#ifndef DEMO_OBJECT_H
#define DEMO_OBJECT_H

#include <string>
#include <iostream>

/**
 * @brief 用于观察构造、拷贝、移动行为的演示类
 *
 * 该类会在构造、拷贝、移动、析构时输出详细信息，
 * 便于理解 std::forward 的行为
 */
class DemoObject {
private:
    std::string data;
    static int instance_count;

public:
    // 构造函数
    explicit DemoObject(const std::string& str);

    // 拷贝构造函数
    DemoObject(const DemoObject& other);

    // 移动构造函数
    DemoObject(DemoObject&& other) noexcept;

    // 拷贝赋值操作符
    DemoObject& operator=(const DemoObject& other);

    // 移动赋值操作符
    DemoObject& operator=(DemoObject&& other) noexcept;

    // 析构函数
    ~DemoObject();

    // 访问器
    [[nodiscard]] const std::string& getData() const;

    // 静态方法
    static int getInstanceCount();
};

/**
 * @brief 用于演示不同参数类型处理的目标函数
 */
namespace TargetFunctions {
    void process_lvalue(DemoObject& obj);
    void process_const_lvalue(const DemoObject& obj);
    void process_rvalue(DemoObject&& obj);
    void process_by_value(const DemoObject& obj);
}

#endif // DEMO_OBJECT_H