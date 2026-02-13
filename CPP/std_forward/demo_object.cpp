#include "demo_object.h"

// 静态成员初始化
int DemoObject::instance_count = 0;

DemoObject::DemoObject(const std::string& str) : data(str) {
    instance_count++;
    std::cout << "[构造] DemoObject(\"" << data << "\") - 实例数: " << instance_count << std::endl;
}

DemoObject::DemoObject(const DemoObject& other) : data(other.data) {
    instance_count++;
    std::cout << "[拷贝构造] DemoObject(\"" << data << "\") - 实例数: " << instance_count << std::endl;
}

DemoObject::DemoObject(DemoObject&& other) noexcept : data(std::move(other.data)) {
    instance_count++;
    std::cout << "[移动构造] DemoObject(\"" << data << "\") - 实例数: " << instance_count << std::endl;
}

DemoObject& DemoObject::operator=(const DemoObject& other) {
    if (this != &other) {
        data = other.data;
        std::cout << "[拷贝赋值] DemoObject = \"" << data << "\"" << std::endl;
    }
    return *this;
}

DemoObject& DemoObject::operator=(DemoObject&& other) noexcept {
    if (this != &other) {
        data = std::move(other.data);
        std::cout << "[移动赋值] DemoObject = \"" << data << "\"" << std::endl;
    }
    return *this;
}

DemoObject::~DemoObject() {
    instance_count--;
    std::cout << "[析构] ~DemoObject(\"" << data << "\") - 实例数: " << instance_count << std::endl;
}

const std::string& DemoObject::getData() const {
    return data;
}

int DemoObject::getInstanceCount() {
    return instance_count;
}

// ================================
// 目标函数实现
// ================================
namespace TargetFunctions {
    void process_lvalue(DemoObject& obj) {
        std::cout << "  → process_lvalue: 处理左值引用 \"" << obj.getData() << "\"" << std::endl;
    }

    void process_const_lvalue(const DemoObject& obj) {
        std::cout << "  → process_const_lvalue: 处理常量左值引用 \"" << obj.getData() << "\"" << std::endl;
    }

    void process_rvalue(DemoObject&& obj) {
        std::cout << "  → process_rvalue: 处理右值引用 \"" << obj.getData() << "\"" << std::endl;
    }

    void process_by_value(const DemoObject& obj) {
        std::cout << "  → process_by_value: 按值处理 \"" << obj.getData() << "\"" << std::endl;
    }
}