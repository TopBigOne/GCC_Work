#pragma once
#include <coroutine>
#include <exception>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

// ─────────────────────────────────────────────────────────────────────────────
// 设计模式：责任链模式 (Chain of Responsibility)
//
// Task<T> 的 continuation 机制：
//   • 每个 Task 知道"完成后该恢复谁"（continuation）
//   • 当内层 Task 完成时，FinalAwaiter 用"对称传输"直接跳到外层 Task，
//     不增加调用栈深度，避免深层链式 co_await 栈溢出
//
// 对称传输（Symmetric Transfer）：
//   await_suspend 返回 coroutine_handle<>，编译器做 tail-call 跳转。
//
// 注意：FinalAwaiter 定义在 promise_type 内部，因为它需要访问 promise_type
//       的完整类型；若放在 Task<T> 层级，promise_type 尚未声明会导致编译错误。
// ─────────────────────────────────────────────────────────────────────────────

namespace m5 {

// ═══════════════════════════════════════════════════════════════════════════
// Task<T>  — 有返回值的异步任务
// ═══════════════════════════════════════════════════════════════════════════
template<typename T>
class Task {
public:
    // ── Promise ───────────────────────────────────────────────────────────
    struct promise_type {
        std::variant<std::monostate, T, std::exception_ptr> result_;
        std::coroutine_handle<> continuation_;

        // FinalAwaiter 定义在 promise_type 内，可完整访问 promise_type
        struct FinalAwaiter {
            bool await_ready() noexcept { return false; }

            // 对称传输：直接跳到等待者，不压新栈帧
            std::coroutine_handle<> await_suspend(
                std::coroutine_handle<promise_type> h) noexcept
            {
                auto cont = h.promise().continuation_;
                return cont ? cont : std::noop_coroutine();
            }
            void await_resume() noexcept {}
        };

        Task get_return_object() noexcept {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }  // 惰性启动
        FinalAwaiter        final_suspend()   noexcept { return {}; }  // 对称传输

        void return_value(T v)         noexcept { result_ = std::move(v); }
        void unhandled_exception()     noexcept { result_ = std::current_exception(); }

        T get_result() {
            if (auto* ep = std::get_if<std::exception_ptr>(&result_))
                std::rethrow_exception(*ep);
            return std::get<T>(std::move(result_));
        }
    };

    // ── Awaiter：实现 co_await task 的行为 ────────────────────────────────
    struct Awaiter {
        std::coroutine_handle<promise_type> handle_;

        bool await_ready() noexcept { return handle_.done(); }

        // 注册 continuation，对称传输切换到本 Task
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept {
            handle_.promise().continuation_ = caller;
            return handle_;
        }

        T await_resume() { return handle_.promise().get_result(); }
    };

    Awaiter operator co_await() noexcept { return Awaiter{handle_}; }

    // ── 生命周期（unique 所有权）──────────────────────────────────────────
    explicit Task(std::coroutine_handle<promise_type> h) noexcept : handle_(h) {}
    Task(const Task&) = delete;
    Task(Task&& o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }
    ~Task() { if (handle_) handle_.destroy(); }

    // sync_wait：单线程场景，resume() 经由对称传输一路执行到链末端再返回
    T sync_wait() {
        handle_.resume();
        return handle_.promise().get_result();
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

// ═══════════════════════════════════════════════════════════════════════════
// Task<void>  — 无返回值（显式特化）
// ═══════════════════════════════════════════════════════════════════════════
template<>
class Task<void> {
public:
    struct promise_type {
        std::optional<std::exception_ptr> exception_;
        std::coroutine_handle<> continuation_;

        struct FinalAwaiter {
            bool await_ready() noexcept { return false; }
            std::coroutine_handle<> await_suspend(
                std::coroutine_handle<promise_type> h) noexcept
            {
                auto cont = h.promise().continuation_;
                return cont ? cont : std::noop_coroutine();
            }
            void await_resume() noexcept {}
        };

        Task get_return_object() noexcept {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        FinalAwaiter        final_suspend()   noexcept { return {}; }

        void return_void()         noexcept {}
        void unhandled_exception() noexcept { exception_ = std::current_exception(); }

        void check_exception() {
            if (exception_ && *exception_)
                std::rethrow_exception(*exception_);
        }
    };

    struct Awaiter {
        std::coroutine_handle<promise_type> handle_;

        bool await_ready() noexcept { return handle_.done(); }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) noexcept {
            handle_.promise().continuation_ = caller;
            return handle_;
        }
        void await_resume() { handle_.promise().check_exception(); }
    };

    Awaiter operator co_await() noexcept { return Awaiter{handle_}; }

    explicit Task(std::coroutine_handle<promise_type> h) noexcept : handle_(h) {}
    Task(const Task&) = delete;
    Task(Task&& o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }
    ~Task() { if (handle_) handle_.destroy(); }

    void sync_wait() {
        handle_.resume();
        handle_.promise().check_exception();
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

// ═══════════════════════════════════════════════════════════════════════════
// 演示协程函数
// ═══════════════════════════════════════════════════════════════════════════

// ── Demo 1: 责任链（多层 co_await）──────────────────────────────────────
inline Task<int> fetch_data(int id) {
    std::cout << "  [fetch_data(" << id << ")] 执行\n";
    co_return id * 10;
}

inline Task<std::string> process(int id) {
    std::cout << "  [process(" << id << ")] 开始，co_await fetch_data\n";
    auto fetcher = fetch_data(id);
    int data = co_await fetcher;
    std::cout << "  [process(" << id << ")] 获得 data=" << data << "\n";
    co_return "result:" + std::to_string(data);
}

inline Task<void> chain_demo() {
    std::cout << "  [chain_demo] 开始\n";
    auto p1 = process(1);
    auto r1 = co_await p1;
    auto p2 = process(2);
    auto r2 = co_await p2;
    std::cout << "  [chain_demo] r1=" << r1 << "  r2=" << r2 << "\n";
    co_return;
}

// ── Demo 2: 异常在责任链中传播 ───────────────────────────────────────────
inline Task<int> may_fail(bool fail) {
    if (fail) throw std::runtime_error("模拟异步错误");
    co_return 42;
}

inline Task<void> exception_demo() {
    std::cout << "  [exception_demo] 调用 may_fail(false)\n";
    auto t1 = may_fail(false);
    int v = co_await t1;
    std::cout << "  [exception_demo] 成功，v=" << v << "\n";

    std::cout << "  [exception_demo] 调用 may_fail(true)\n";
    try {
        auto t2 = may_fail(true);
        co_await t2;
    } catch (const std::exception& e) {
        // 异常路径：may_fail → unhandled_exception 存储 → FinalAwaiter 传回
        // → Awaiter::await_resume 重新抛出 → 被这里捕获
        std::cout << "  [exception_demo] 捕获: " << e.what() << "\n";
    }
    co_return;
}

// ── Demo 3: 深层嵌套（验证对称传输无栈溢出）─────────────────────────────
inline Task<int> level(int depth) {
    if (depth == 0) {
        std::cout << "  [level(0)] 底层，返回 1\n";
        co_return 1;
    }
    auto sub_task = level(depth - 1);
    int sub = co_await sub_task;
    co_return sub + 1;
}

inline Task<void> deep_chain_demo(int depth) {
    auto t = level(depth);
    int result = co_await t;
    std::cout << "  [deep_chain] depth=" << depth << "  result=" << result << "\n";
    co_return;
}

// ───────────────────────────────────────────────────────────────────────────
inline void run_module5() {
    std::cout << "\n╔══ M5 Task<T>（责任链模式）══════════════════╗\n";

    std::cout << "\n── Demo 1: 责任链 co_await 链式调用 ──\n";
    chain_demo().sync_wait();

    std::cout << "\n── Demo 2: 异常在链中传播并被捕获 ──\n";
    exception_demo().sync_wait();

    std::cout << "\n── Demo 3: 5 层嵌套（对称传输，无栈溢出）──\n";
    deep_chain_demo(5).sync_wait();

    std::cout << "╚═════════════════════════════════════════════╝\n";
}

} // namespace m5
