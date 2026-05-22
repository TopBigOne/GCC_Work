#pragma once
/**
 * DemoLifetime.h — 模块4：生命周期陷阱
 *
 * string_view 不持有内存，不延长所引用对象的生命周期。
 * 本模块演示：
 *   - 安全用法：view 的生命周期 < 原始数据的生命周期
 *   - 陷阱说明：将临时 string 赋给 string_view（注释展示，不触发 UB）
 *   - 安全转回 std::string 的方式
 *   - 函数返回 string_view 的正确姿势
 */
#include "IDemo.h"

class DemoLifetime : public IDemo {
public:
    std::string name()        const override;
    std::string description() const override;
    void        run()               override;
};
