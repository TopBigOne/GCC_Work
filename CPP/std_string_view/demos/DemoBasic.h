#pragma once
/**
 * DemoBasic.h — 模块1：基本概念与构造
 *
 * 演示 std::string_view 的本质和各种构造方式：
 *   - 从字符串字面量构造
 *   - 从 std::string 构造
 *   - 从 char[] 构造
 *   - 带长度的范围构造
 *   - 基本属性访问：size/empty/front/back/operator[]
 *   - 注意：data() 不保证 null-terminated
 */
#include "IDemo.h"

class DemoBasic : public IDemo {
public:
    std::string name()        const override;
    std::string description() const override;
    void        run()               override;
};
