#pragma once
#include <coroutine>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// 设计模式：模板方法模式 (Template Method Pattern)
//
// 框架（编译器）定义协程生命周期的算法骨架，promise_type 的各个方法是"钩子"。
// 通过组合不同的钩子，得到行为完全不同的三种协程类型：
//
//   initial_suspend  × final_suspend  → 协程行为
//   suspend_never    × suspend_never  → FireAndForget（热启动，帧立即销毁，返回对象无 handle）
//   suspend_always   × suspend_always → LazyTask    （惰性启动，析构时由 handle.destroy() 销毁）
//   suspend_always   × suspend_always → ValueTask<T>（同上，额外在帧里存结果，get() 读取后析构）
//
// ⚠️  final_suspend = suspend_never 的陷阱：
//     帧在协程结束后被 runtime 立即 free，此后 handle 是野指针。
//     只有"永远不再碰 handle"的 FireAndForget（返回对象里根本没有 handle 字段）
//     才能安全使用 suspend_never。其余带 handle 字段的类型一律用 suspend_always。
// ─────────────────────────────────────────────────────────────────────────────

namespace m2 {

// ═══════════════════════════════════════════════════════════════════════════
// Demo 1: FireAndForget
// 钩子组合：initial=suspend_never / final=suspend_never
// 行为：调用即运行，返回时已完成，帧自动销毁，调用者无需管理。
// 适合：日志、埋点等不关心结果的后台操作。
// ═══════════════════════════════════════════════════════════════════════════
struct FireAndForget {
    struct promise_type {
        FireAndForget get_return_object() noexcept { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; }  // 钩子：热启动
        std::suspend_never final_suspend() noexcept { return {}; }    // 钩子：自动销毁
        void return_void() noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };
};

inline FireAndForget fire_demo() {
    std::cout << "  [FireAndForget] 执行中（调用者已拿到返回值）\n";
    co_return;
}

// ═══════════════════════════════════════════════════════════════════════════
// Demo 2: LazyTask
// 钩子组合：initial=suspend_always / final=suspend_always
// 行为：创建时不运行，需调用 start() 触发；析构时由 ~LazyTask() 统一销毁帧。
// 适合：需要延迟启动、按需触发的任务。
//
// ⚠️  为什么 final 也用 suspend_always？
//     若改成 suspend_never，runtime 在 co_return 后立即 free 帧，
//     此后析构里再调 handle_.done() 就是野指针访问 → SIGBUS。
//     只要类里保存了 handle_，final 就必须用 suspend_always。
// ═══════════════════════════════════════════════════════════════════════════
class LazyTask {
public:
    struct promise_type {
        LazyTask get_return_object() noexcept {
            return LazyTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }  // 钩子：惰性启动
        std::suspend_always final_suspend()   noexcept { return {}; }  // 钩子：帧由析构统一销毁
        void return_void() noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };

    explicit LazyTask(std::coroutine_handle<promise_type> h) noexcept : handle_(h) {}
    LazyTask(const LazyTask&) = delete;
    LazyTask(LazyTask&& o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }
    ~LazyTask() {
        // final=suspend_always 保证帧一直有效，可以安全 destroy
        // 无论 start() 是否被调用，handle_ 要么有效要么为 nullptr（被移走后）
        if (handle_) handle_.destroy();
    }

    void start() { handle_.resume(); }

private:
    std::coroutine_handle<promise_type> handle_;
};

inline LazyTask lazy_demo(int id) {
    std::cout << "  [LazyTask #" << id << "] start() 触发后才执行到这里\n";
    co_return;
}

// ═══════════════════════════════════════════════════════════════════════════
// Demo 3: ValueTask<T>
// 钩子组合：initial=suspend_always / final=suspend_always
// 行为：惰性启动；结束后帧保留，调用者读完结果再手动销毁（析构时自动）。
// 适合：需要在协程完成后读取返回值的场景。
// ═══════════════════════════════════════════════════════════════════════════
template<typename T>
class ValueTask {
public:
    struct promise_type {
        T value{};

        ValueTask get_return_object() noexcept {
            return ValueTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }  // 钩子：惰性启动
        std::suspend_always final_suspend() noexcept { return {}; }    // 钩子：保留帧

        void return_value(T v) noexcept { value = std::move(v); }     // 钩子：存储结果
        void unhandled_exception() noexcept { std::terminate(); }
    };

    explicit ValueTask(std::coroutine_handle<promise_type> h) noexcept : handle_(h) {}
    ValueTask(const ValueTask&) = delete;
    ValueTask(ValueTask&& o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }
    ~ValueTask() { if (handle_) handle_.destroy(); }

    // 驱动协程运行到完成，返回结果
    T get() {
        handle_.resume();
        return handle_.promise().value;
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

inline ValueTask<int> value_demo(int x) {
    std::cout << "  [ValueTask<int>] 计算 " << x << " ^ 2\n";
    co_return x * x;
}

inline ValueTask<std::string> string_value_demo(std::string prefix) {
    std::cout << "  [ValueTask<string>] 拼接字符串\n";
    co_return prefix + "_result";
}

// ───────────────────────────────────────────────────────────────────────────
inline void run_module2() {
    std::cout << "\n╔══ M2 Promise 类型（模板方法模式）═══════════╗\n";

    std::cout << "\n── Demo 1: FireAndForget（调用即运行）──\n";
    fire_demo();  // 已在调用时完成
    std::cout << "  主线程: fire_demo() 调用后协程已执行完毕\n";

    std::cout << "\n── Demo 2: LazyTask（惰性，手动 start）──\n";
    {
        auto t1 = lazy_demo(1);
        auto t2 = lazy_demo(2);
        std::cout << "  主线程: 两个 LazyTask 已创建，均未执行\n";
        t2.start();  // 可以乱序启动
        t1.start();
    }

    std::cout << "\n── Demo 3: ValueTask<T>（有返回值）──\n";
    {
        auto vt = value_demo(7);
        int r = vt.get();
        std::cout << "  [ValueTask<int>] 结果: " << r << "\n";

        auto st  = string_value_demo("hello");
        auto str = st.get();  // 先驱动协程，再打印结果，避免输出交叉
        std::cout << "  [ValueTask<string>] 结果: " << str << "\n";
    }

    std::cout << "╚═════════════════════════════════════════════╝\n";
}

} // namespace m2
