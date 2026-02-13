/**
 * @file main.cpp
 * @brief std::forward 完美转发学习演示程序
 *
 * 这个程序演示了 std::forward 的各种使用场景和常见陷阱
 * 通过模块化的代码结构展示完美转发的工作原理
 *
 * @author Learning Demo
 * @version 1.0
 */

#include "forward_demos.h"
#include <iostream>

/**
 * @brief 主函数 - 程序入口点
 *
 * 按顺序执行各个演示模块，展示 std::forward 的不同用法
 * 每个演示模块独立运行，便于理解和调试
 *
 * @return int 程序退出状态码
 */
int main() {
    std::cout << "std::forward 完美转发演示程序" << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << "本程序将演示 std::forward 的各种使用场景\n" << std::endl;

    try {
        // 1. 基础对比演示：展示使用和不使用 std::forward 的区别
        ForwardDemos::run_basic_comparison_demo();

        // 2. 函数对象转发演示：展示如何转发函数和参数
        ForwardDemos::run_function_forwarding_demo();

        // 3. 工厂函数演示：展示完美转发在对象创建中的应用
        ForwardDemos::run_factory_demo();

        // 4. 容器 emplace 演示：展示在容器中直接构造对象
        ForwardDemos::run_emplace_demo();

        // 5. 引用折叠规则演示：理解模板类型推导机制
        ForwardDemos::run_reference_collapsing_demo();

        // 6. 常见陷阱演示：避免错误使用 std::forward
        ForwardDemos::run_trap_demo();

        // 7. 显示最终统计信息
        ForwardDemos::show_final_statistics();

        std::cout << "\n程序即将结束，观察对象析构..." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "程序执行出错: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}