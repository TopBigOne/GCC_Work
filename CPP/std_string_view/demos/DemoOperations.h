#pragma once
/**
 * DemoOperations.h — 模块2：常用操作
 *
 * 演示 std::string_view 的常用成员函数：
 *   - substr(pos, len)       — 零拷贝子串
 *   - find / rfind           — 正向/反向查找
 *   - find_first_of / find_last_of
 *   - starts_with / ends_with（C++20）
 *   - remove_prefix / remove_suffix — 原地收缩视图
 *   - 迭代器遍历
 */
#include "IDemo.h"

class DemoOperations : public IDemo {
public:
    std::string name()        const override;
    std::string description() const override;
    void        run()               override;
};
