#include "scope_guard.h"
#include <cstdio>
#include <iostream>
#include <stdexcept>

namespace raii {
namespace scope {

// ---- ScopeGuard ----

ScopeGuard::ScopeGuard(Action action) noexcept
    : action_(std::move(action)), dismissed_(false) {}

ScopeGuard::~ScopeGuard() noexcept {
    if (!dismissed_) {
        try { action_(); } catch (...) {}
    }
}

void ScopeGuard::dismiss() noexcept { dismissed_ = true; }

ScopeGuard::ScopeGuard(ScopeGuard&& other) noexcept
    : action_(std::move(other.action_)), dismissed_(other.dismissed_) {
    other.dismissed_ = true;
}

// ---- ScopeOnFailure ----

ScopeOnFailure::ScopeOnFailure(Action action) noexcept
    : action_(std::move(action))
    , exception_count_(std::uncaught_exceptions()) {}

ScopeOnFailure::~ScopeOnFailure() noexcept {
    if (std::uncaught_exceptions() > exception_count_) {
        try { action_(); } catch (...) {}
    }
}

// ---- ScopeOnSuccess ----

ScopeOnSuccess::ScopeOnSuccess(Action action) noexcept
    : action_(std::move(action))
    , exception_count_(std::uncaught_exceptions()) {}

ScopeOnSuccess::~ScopeOnSuccess() noexcept {
    if (std::uncaught_exceptions() == exception_count_) {
        try { action_(); } catch (...) {}
    }
}

// ---- demo ----

void demo_scope_guard() {
    std::cout << "\n====== 模块5：ScopeGuard ======\n";

    std::cout << "\n[基础 ScopeGuard]\n";
    {
        FILE* f = std::fopen("/tmp/scope_test.txt", "w+");
        std::cout << "  打开文件\n";
        auto guard = make_scope_guard([f]() {
            std::fclose(f);
            std::cout << "  [ScopeGuard] 自动关闭文件\n";
        });
        std::fputs("scope guard test\n", f);
        std::cout << "  写入完成，离开作用域...\n";
    }

    std::cout << "\n[dismiss — 事务提交跳过回滚]\n";
    {
        std::cout << "  开始事务\n";
        bool committed = false;
        auto rollback = make_scope_guard([&]() {
            if (!committed) std::cout << "  [ScopeGuard] 执行回滚！\n";
        });
        committed = true;
        rollback.dismiss();
        std::cout << "  事务提交成功，dismiss 跳过回滚\n";
    }

    std::cout << "\n[dismiss — 事务失败触发回滚]\n";
    {
        auto rollback = make_scope_guard([]() {
            std::cout << "  [ScopeGuard] 执行回滚（未 dismiss）\n";
        });
        std::cout << "  模拟操作失败，离开作用域时自动回滚...\n";
    }

    std::cout << "\n[ScopeOnFailure / ScopeOnSuccess]\n";
    {
        ScopeOnSuccess on_ok([]() { std::cout << "  [OnSuccess] 正常提交\n"; });
        ScopeOnFailure on_fail([]() { std::cout << "  [OnFailure] 异常回滚\n"; });
        std::cout << "  正常执行完毕...\n";
    }

    std::cout << "\n[ScopeOnFailure 异常场景]\n";
    try {
        ScopeOnSuccess on_ok([]() { std::cout << "  [OnSuccess] 不应执行\n"; });
        ScopeOnFailure on_fail([]() { std::cout << "  [OnFailure] 异常回滚触发\n"; });
        throw std::runtime_error("模拟异常");
    } catch (const std::exception& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
    }

    std::cout << "\n[SCOPE_EXIT 宏]\n";
    {
        std::cout << "  申请资源\n";
        SCOPE_EXIT {
            std::cout << "  [SCOPE_EXIT] 自动清理\n";
        };
        std::cout << "  使用资源...\n";
    }

    std::cout << "====== 模块5 结束 ======\n";
}

}  // namespace scope
}  // namespace raii
