#ifndef FORWARD_DEMOS_H
#define FORWARD_DEMOS_H

#include "demo_object.h"
#include <utility>
#include <vector>
#include <memory>

/**
 * @brief std::forward 完美转发演示模块
 *
 * 包含各种 std::forward 使用场景的演示函数
 */
namespace ForwardDemos {

    // ================================
    // 基础演示函数
    // ================================

    /**
     * @brief 错误示例：不使用 std::forward
     */
    template<typename T>
    void bad_wrapper(T&& param);

    /**
     * @brief 正确示例：使用 std::forward 完美转发
     */
    template<typename T>
    void good_wrapper(T&& param);

    // ================================
    // 高级演示函数
    // ================================

    /**
     * @brief 函数对象的完美转发
     */
    template<typename Func, typename... Args>
    auto perfect_forwarder(Func&& func, Args&&... args)
        -> decltype(std::forward<Func>(func)(std::forward<Args>(args)...));

    /**
     * @brief 工厂函数示例
     */
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique_perfect(Args&&... args);

    /**
     * @brief 容器的 emplace 风格函数
     */
    template<typename... Args>
    void emplace_demo(std::vector<DemoObject>& vec, Args&&... args);

    /**
     * @brief 演示引用折叠规则
     */
    template<typename T>
    void reference_collapsing_demo(T&& param);

    /**
     * @brief 常见陷阱演示
     */
    template<typename T>
    void multiple_forward_trap(T&& param);

    // ================================
    // 演示控制函数
    // ================================

    /**
     * @brief 运行基础对比演示
     */
    void run_basic_comparison_demo();

    /**
     * @brief 运行函数对象转发演示
     */
    void run_function_forwarding_demo();

    /**
     * @brief 运行工厂函数演示
     */
    void run_factory_demo();

    /**
     * @brief 运行容器 emplace 演示
     */
    void run_emplace_demo();

    /**
     * @brief 运行引用折叠规则演示
     */
    void run_reference_collapsing_demo();

    /**
     * @brief 运行常见陷阱演示
     */
    void run_trap_demo();

    /**
     * @brief 显示最终统计
     */
    void show_final_statistics();

    // ================================
    // 辅助函数
    // ================================

    /**
     * @brief 打印分隔线
     */
    void print_separator(const std::string& title);

} // namespace ForwardDemos

// ================================
// 模板函数的实现（必须在头文件中）
// ================================

template<typename T>
void ForwardDemos::bad_wrapper(T&& param) {
    std::cout << "bad_wrapper: 参数总是被当作左值传递" << std::endl;
    // 这里 param 总是左值，即使传入的是右值
    TargetFunctions::process_by_value(param);  // 总是触发拷贝构造
}

template<typename T>
void ForwardDemos::good_wrapper(T&& param) {
    std::cout << "good_wrapper: 使用 std::forward 保持值类别" << std::endl;
    // 使用 std::forward 保持原始值类别
    TargetFunctions::process_by_value(std::forward<T>(param));
}

template<typename Func, typename... Args>
auto ForwardDemos::perfect_forwarder(Func&& func, Args&&... args)
    -> decltype(std::forward<Func>(func)(std::forward<Args>(args)...)) {
    std::cout << "perfect_forwarder: 转发函数和参数" << std::endl;
    return std::forward<Func>(func)(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
std::unique_ptr<T> ForwardDemos::make_unique_perfect(Args&&... args) {
    std::cout << "make_unique_perfect: 完美转发构造参数" << std::endl;
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename... Args>
void ForwardDemos::emplace_demo(std::vector<DemoObject>& vec, Args&&... args) {
    std::cout << "emplace_demo: 直接在容器中构造对象" << std::endl;
    vec.emplace_back(std::forward<Args>(args)...);
}

template<typename T>
void ForwardDemos::reference_collapsing_demo(T&& param) {
    std::cout << "引用折叠演示:" << std::endl;

    if constexpr (std::is_lvalue_reference_v<T>) {
        std::cout << "  T 是左值引用类型, T&& 折叠为 T&" << std::endl;
    } else {
        std::cout << "  T 是普通类型, T&& 是右值引用" << std::endl;
    }

    if constexpr (std::is_lvalue_reference_v<T&&>) {
        std::cout << "  参数是左值引用" << std::endl;
    } else {
        std::cout << "  参数是右值引用" << std::endl;
    }

    // 为了消除未使用参数的警告
    (void)param;
}

template<typename T>
void ForwardDemos::multiple_forward_trap(T&& param) {
    std::cout << "⚠️  多次 forward 的陷阱演示:" << std::endl;

    // 第一次使用 - 正常
    TargetFunctions::process_const_lvalue(std::forward<T>(param));

    // 第二次使用 - 危险！如果 T 是右值类型，param 可能已被移动
    // TargetFunctions::process_const_lvalue(std::forward<T>(param));  // 取消注释查看效果
    std::cout << "  第二次使用可能访问被移动的对象（已注释掉）" << std::endl;
}

#endif // FORWARD_DEMOS_H