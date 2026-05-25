#include "demo_chain.h"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

// 责任链模式：多层处理器依次尝试处理异常，处理不了则向上传递
class ExceptionHandler {
public:
    // 显示调用构造函数
    explicit ExceptionHandler(std::string name) : name_(std::move(name)) {}
    virtual ~ExceptionHandler() = default;

    void set_next(ExceptionHandler* next) { next_ = next; }

    void handle(std::exception_ptr ep) {
        if (try_handle(ep)) return;
        if (next_) {
            next_->handle(ep);
        } else {
            std::cerr << "  [未处理] 异常无人接手\n";
        }
    }

protected:
    virtual bool try_handle(std::exception_ptr ep) = 0;
    std::string name_;

private:
    ExceptionHandler* next_ = nullptr;
};

// 只处理 runtime_error
class RuntimeErrorHandler : public ExceptionHandler {
public:
    // 是 C++11 的继承构造函数语法。意思是：把父类 ExceptionHandler 的所有构造函数原样继承到子类，不需要在子类里重复写一遍
    using ExceptionHandler::ExceptionHandler;

protected:
    bool try_handle(std::exception_ptr ep) override {
        try {
            std::rethrow_exception(ep);
        } catch (const std::runtime_error& e) {
            std::cout << "  [" << name_ << "] 处理 runtime_error: " << e.what() << "\n";
            return true;
        } catch (...) {
            return false;
        }
    }
};

// 兜底：处理所有 std::exception
class FallbackHandler : public ExceptionHandler {
public:
    // 是 C++11 的继承构造函数语法。
    // 意思是：把父类 ExceptionHandler 的所有构造函数原样继承到子类，不需要在子类里重复写一遍
    using ExceptionHandler::ExceptionHandler;

protected:
    bool try_handle(std::exception_ptr ep) override {
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            std::cout << "  [" << name_ << "] 兜底处理: " << e.what() << "\n";
            return true;
        } catch (...) {
            return false;
        }
    }
};

void DemoChain::run() {
    std::cout << "\n[demo_chain] 异常责任链\n";

    RuntimeErrorHandler runtime_handler("RuntimeHandler");
    FallbackHandler fallback_handler("FallbackHandler");
    runtime_handler.set_next(&fallback_handler);

    auto ep1 = std::make_exception_ptr(std::runtime_error("网络断开"));
    auto ep2 = std::make_exception_ptr(std::logic_error("非法状态"));

    runtime_handler.handle(ep1); // RuntimeHandler 处理
    runtime_handler.handle(ep2); // 传递到 FallbackHandler
}
