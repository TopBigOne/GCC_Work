#pragma once
/**
 * IDemo.h
 * 设计模式：Template Method（模板方法）
 *
 * 定义 demo 的统一接口：
 *   name()        — demo 名称
 *   description() — 功能说明
 *   run()         — 执行 demo（由子类实现）
 *
 * main.cpp 通过多态统一调用所有 demo，
 * 无需关心每个 demo 的具体实现。
 */
#include <string>

class IDemo {
public:
    virtual ~IDemo() = default;

    virtual std::string name()        const = 0;
    virtual std::string description() const = 0;
    virtual void        run()               = 0;
};
