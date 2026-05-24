#pragma once
#include <exception>
#include <functional>
#include <utility>

// ============================================================
// 模块5：ScopeGuard — 通用延迟清理（类似 Go 的 defer）
// 知识点：
//   1. 用 lambda 捕获任意清理动作，离开作用域时自动执行
//   2. dismiss() 可取消清理（用于提交成功后跳过回滚）
//   3. 区分正常退出 vs 异常退出（OnFailure / OnSuccess 变体）
//   4. SCOPE_EXIT 宏：最简洁的用法
// ============================================================

namespace raii {
namespace scope {

// ----------------------------------------------------------
// 5.1 ScopeGuard — 声明（实现在 .cpp）
// ----------------------------------------------------------
class ScopeGuard {
public:
    using Action = std::function<void()>;

    explicit ScopeGuard(Action action) noexcept;
    ~ScopeGuard() noexcept;
    void dismiss() noexcept;

    ScopeGuard(const ScopeGuard&)            = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept;

private:
    Action action_;
    bool   dismissed_;
};

// ----------------------------------------------------------
// 5.2 make_scope_guard — 函数模板，必须在头文件
// ----------------------------------------------------------
template <typename F>
[[nodiscard]] ScopeGuard make_scope_guard(F&& f) {
    return ScopeGuard(std::forward<F>(f));
}

// ----------------------------------------------------------
// 5.3 异常感知变体（声明在此，实现在 .cpp）
// ----------------------------------------------------------
class ScopeOnFailure {
public:
    using Action = std::function<void()>;
    explicit ScopeOnFailure(Action action) noexcept;
    ~ScopeOnFailure() noexcept;
    ScopeOnFailure(const ScopeOnFailure&)            = delete;
    ScopeOnFailure& operator=(const ScopeOnFailure&) = delete;
private:
    Action action_;
    int    exception_count_;
};

class ScopeOnSuccess {
public:
    using Action = std::function<void()>;
    explicit ScopeOnSuccess(Action action) noexcept;
    ~ScopeOnSuccess() noexcept;
    ScopeOnSuccess(const ScopeOnSuccess&)            = delete;
    ScopeOnSuccess& operator=(const ScopeOnSuccess&) = delete;
private:
    Action action_;
    int    exception_count_;
};

// ----------------------------------------------------------
// 5.4 SCOPE_EXIT 宏辅助（operator+ 是模板，必须在头文件）
// ----------------------------------------------------------
namespace detail {
struct ScopeGuardTag {};

template <typename F>
ScopeGuard operator+(ScopeGuardTag, F&& f) {
    return make_scope_guard(std::forward<F>(f));
}
}  // namespace detail

#define SCOPE_EXIT                                                          \
    auto SCOPE_GUARD_VAR_(__scope_guard_, __LINE__) =                       \
        ::raii::scope::detail::ScopeGuardTag{} + [&]() noexcept

#define SCOPE_GUARD_VAR_(a, b) SCOPE_GUARD_CONCAT_(a, b)
#define SCOPE_GUARD_CONCAT_(a, b) a##b

void demo_scope_guard();

}  // namespace scope
}  // namespace raii
