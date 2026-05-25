#include "demo_basic.h"

#include <exception>
#include <iostream>
#include <stdexcept>

// 演示 current_exception + rethrow_exception
static void capture_and_rethrow() {
    std::cout << "\n[demo_basic] capture_and_rethrow\n";

    std::exception_ptr ep;

    try {
        throw std::runtime_error("原始异常");
    } catch (...) {
        ep = std::current_exception();
    }

    std::cout << "  异常已保存，程序继续运行...\n";

    if (ep) {
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            std::cout << "  延迟捕获: " << e.what() << "\n";
        }
    }
}

// 演示 make_exception_ptr（无需 throw/catch）
static void make_without_throw() {
    std::cout << "\n[demo_basic] make_without_throw\n";

    auto ep = std::make_exception_ptr(std::logic_error("直接构造的异常"));

    try {
        std::rethrow_exception(ep);
    } catch (const std::exception& e) {
        std::cout << "  捕获: " << e.what() << "\n";
    }
}

void DemoBasic::run() {
    capture_and_rethrow();
    make_without_throw();
}
