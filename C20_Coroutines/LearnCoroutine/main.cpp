/**
 * C++20 协程学习 —— 统一入口
 *
 * 直接在 CLion 中 Run 即可按顺序运行所有模块示例。
 *
 * 模块列表：
 *   M0  C++11/17 前置基础  (lambda / function / move / template / RAII / optional / variant)
 *   M1  协程基础概念       (SimpleTask / ManualTask / StepTask)
 *   M2  Promise 类型       (FireAndForget / LazyTask / ValueTask<T>)     [模板方法模式]
 *   M3  Generator          (fibonacci / range / map_gen / filter_gen)   [迭代器模式]
 *   M4  Awaitable          (Ready / Conditional / Manual / Thread)       [策略模式]
 *   M5  Task<T>            (责任链 / 异常传播 / 嵌套 Task)               [责任链模式]
 *   M6  实战应用           (协作调度 / Event / Channel)                  [Reactor+观察者]
 */

#include "module0_prerequisites/prerequisites.h"
#include "module1_basics/basics.h"
#include "module2_promise/promise.h"
#include "module3_generator/generator.h"
#include "module4_awaitable/awaitable.h"
#include "module5_task/task.h"
#include "module6_realworld/realworld.h"

int main() {
    std::cout << "╔═════════════════════════════════════════╗\n";
    std::cout << "║    C++20 协程学习 —— 全模块演示-v2         ║\n";
    std::cout << "╚═════════════════════════════════════════╝\n";

    m0::run_module0();
    m1::run_module1();
    m2::run_module2();
    m3::run_module3();
    m4::run_module4();
    m5::run_module5();
    m6::run_module6();

    std::cout << "\n✅ 全部模块演示完成\n";
    return 0;
}
