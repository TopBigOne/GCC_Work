#include "forward_demos.h"
#include <iostream>

namespace ForwardDemos {

    void run_basic_comparison_demo() {
        print_separator("1. 对比：不使用 vs 使用 std::forward");

        std::cout << "\n--- 传递右值 ---" << std::endl;
        std::cout << "bad_wrapper(DemoObject(\"temp\")):" << std::endl;
        bad_wrapper(DemoObject("temp"));

        std::cout << "\ngood_wrapper(DemoObject(\"temp\")):" << std::endl;
        good_wrapper(DemoObject("temp"));

        std::cout << "\n--- 传递左值 ---" << std::endl;
        DemoObject obj("persistent");
        std::cout << "bad_wrapper(obj):" << std::endl;
        bad_wrapper(obj);

        std::cout << "\ngood_wrapper(obj):" << std::endl;
        good_wrapper(obj);
    }

    void run_function_forwarding_demo() {
        print_separator("2. 函数对象的完美转发");

        auto lambda = [](const DemoObject& o) {
            std::cout << "  Lambda处理: " << o.getData() << std::endl;
            return o.getData().length();
        };

        std::cout << "使用 perfect_forwarder 调用 lambda:" << std::endl;
        size_t result = perfect_forwarder(lambda, DemoObject("lambda_test"));
        std::cout << "返回值: " << result << std::endl;
    }

    void run_factory_demo() {
        print_separator("3. 工厂函数演示");

        std::cout << "使用 make_unique_perfect 创建对象:" << std::endl;
        auto unique_obj = make_unique_perfect<DemoObject>("factory_created");
        std::cout << "创建的对象数据: " << unique_obj->getData() << std::endl;
    }

    void run_emplace_demo() {
        print_separator("4. 容器 emplace 演示");

        std::vector<DemoObject> vec;
        std::cout << "直接在 vector 中构造对象:" << std::endl;
        emplace_demo(vec, "emplace_1");
        emplace_demo(vec, "emplace_2");

        std::cout << "Vector 内容:" << std::endl;
        for (const auto& item : vec) {
            std::cout << "  " << item.getData() << std::endl;
        }
    }

    void run_reference_collapsing_demo() {
        print_separator("5. 引用折叠规则演示");

        DemoObject test_obj("ref_test");
        std::cout << "传递左值:" << std::endl;
        reference_collapsing_demo(test_obj);

        std::cout << "\n传递右值:" << std::endl;
        reference_collapsing_demo(DemoObject("temp_ref"));
    }

    void run_trap_demo() {
        print_separator("6. 常见陷阱演示");

        std::cout << "多次 forward 的陷阱:" << std::endl;
        multiple_forward_trap(DemoObject("trap_demo"));
    }

    void show_final_statistics() {
        print_separator("7. 最终对象统计");
        std::cout << "当前存活的 DemoObject 实例数: " << DemoObject::getInstanceCount() << std::endl;
    }

    void print_separator(const std::string& title) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << title << std::endl;
        std::cout << std::string(50, '=') << std::endl;
    }

} // namespace ForwardDemos