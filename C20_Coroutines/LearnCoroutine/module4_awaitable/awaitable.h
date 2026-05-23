#pragma once
#include <coroutine>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

// ─────────────────────────────────────────────────────────────────────────────
// 设计模式：策略模式 (Strategy Pattern)
//
// co_await expr 时，编译器查询三个方法决定行为：
//   await_ready()   → bool               是否跳过挂起？
//   await_suspend() → void/bool/handle   挂起时做什么？
//   await_resume()  → T                  恢复后 co_await 的求值结果
//
// 不同的 Awaitable 实现 = 不同的"等待策略"，可自由组合。
// ─────────────────────────────────────────────────────────────────────────────

namespace m4 {

// ── 辅助 Task 类型（用于承载 M4 各种 Awaitable 演示）──────────────────────
// final_suspend = suspend_always，让调用者通过 done() 检查状态并手动销毁。
class DemoTask {
public:
    struct promise_type {
        DemoTask get_return_object() noexcept {
            return DemoTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend()   noexcept { return {}; }
        void return_void()         noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit DemoTask(std::coroutine_handle<promise_type> h) noexcept : handle(h) {}
    DemoTask(const DemoTask&) = delete;
    DemoTask(DemoTask&& o) noexcept : handle(o.handle) { o.handle = nullptr; }
    ~DemoTask() { if (handle) handle.destroy(); }

    bool done()   const noexcept { return !handle || handle.done(); }
    void resume()       noexcept { if (!done()) handle.resume(); }
};

// ── 异步完成通知 Task（用于跨线程演示）────────────────────────────────────
// final_suspend 完成后通过 std::promise 通知主线程，并自行销毁帧。
class AsyncTask {
public:
    struct promise_type {
        std::promise<void>* completion = nullptr;

        AsyncTask get_return_object() noexcept {
            return AsyncTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }

        struct FinalNotifier {
            bool await_ready() noexcept { return false; }
            void await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                if (h.promise().completion)
                    h.promise().completion->set_value();
                h.destroy();  // 自行销毁帧
            }
            void await_resume() noexcept {}
        };
        FinalNotifier final_suspend() noexcept { return {}; }
        void return_void()         noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };

    // 帧由 final_suspend 自行销毁，析构时不需要操作
    std::coroutine_handle<promise_type> handle;
    explicit AsyncTask(std::coroutine_handle<promise_type> h) noexcept : handle(h) {}
    AsyncTask(const AsyncTask&) = delete;
    AsyncTask(AsyncTask&&) = delete;

    void start(std::promise<void>& p) {
        handle.promise().completion = &p;
        handle.resume();
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 策略 1: ReadyAwaitable — await_ready = true，永不挂起
// 场景：结果已就绪，无需任何等待。
// ═══════════════════════════════════════════════════════════════════════════
struct ReadyAwaitable {
    bool await_ready() noexcept {
        std::cout << "  [ReadyAwaitable] await_ready → true，跳过挂起\n";
        return true;
    }
    void await_suspend(std::coroutine_handle<>) noexcept {}  // 不会被调用
    int  await_resume() noexcept {
        std::cout << "  [ReadyAwaitable] await_resume — 直接到达，值 = 42\n";
        return 42;
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 策略 2: ConditionalAwaitable — await_suspend 返回 bool，条件挂起
// 场景：根据运行时状态决定是否真正挂起。
// ═══════════════════════════════════════════════════════════════════════════
struct ConditionalAwaitable {
    bool ready_;  // 外部传入的"结果是否就绪"标志

    bool await_ready() noexcept { return false; }  // 先进入 await_suspend

    bool await_suspend(std::coroutine_handle<>) noexcept {
        if (ready_) {
            std::cout << "  [ConditionalAwaitable] 条件满足 → await_suspend 返回 false（不挂起）\n";
            return false;  // false = 不挂起，立刻 await_resume
        }
        std::cout << "  [ConditionalAwaitable] 条件不满足 → await_suspend 返回 true（挂起）\n";
        return true;  // true = 挂起，需外部 resume
    }

    void await_resume() noexcept {
        std::cout << "  [ConditionalAwaitable] await_resume\n";
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 策略 3: ManualAwaitable — 将句柄暴露给外部，由外部决定何时 resume
// 场景：回调式异步（如 IO 完成回调），句柄由调度器或事件循环持有。
// ═══════════════════════════════════════════════════════════════════════════
struct ManualAwaitable {
    std::coroutine_handle<>* out_;  // 将句柄写到这里，供外部调用 resume

    bool await_ready() noexcept { return false; }

    void await_suspend(std::coroutine_handle<> h) noexcept {
        std::cout << "  [ManualAwaitable] 协程挂起，句柄已写到 *out_\n";
        *out_ = h;
    }

    void await_resume() noexcept {
        std::cout << "  [ManualAwaitable] 外部调用 resume 后协程恢复\n";
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 策略 4: ThreadAwaitable — 在后台线程 sleep 后 resume，模拟真实异步
// await_suspend 返回 void：无条件挂起，将 resume 投递给后台线程。
// ═══════════════════════════════════════════════════════════════════════════
struct ThreadAwaitable {
    int delay_ms;

    bool await_ready() noexcept { return false; }

    void await_suspend(std::coroutine_handle<> h) noexcept {
        // 启动后台线程，sleep 后 resume 协程
        std::thread([h, d = delay_ms]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(d));
            std::cout << "  [ThreadAwaitable] 后台线程 sleep " << d
                      << "ms 后调用 h.resume()\n";
            h.resume();
        }).detach();
    }

    void await_resume() noexcept {
        std::cout << "  [ThreadAwaitable] 协程恢复（由后台线程触发）\n";
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// 演示协程函数
// ═══════════════════════════════════════════════════════════════════════════

inline DemoTask demo_ready() {
    std::cout << "  协程: co_await ReadyAwaitable{}\n";
    int val = co_await ReadyAwaitable{};
    std::cout << "  协程: 继续执行，val = " << val << "\n";
    co_return;
}

inline DemoTask demo_conditional(bool flag) {
    std::cout << "  协程: co_await ConditionalAwaitable{ready=" << std::boolalpha << flag << "}\n";
    co_await ConditionalAwaitable{flag};
    std::cout << "  协程: 执行完毕\n";
    co_return;
}

inline DemoTask demo_manual(std::coroutine_handle<>& out) {
    std::cout << "  协程: co_await ManualAwaitable\n";
    co_await ManualAwaitable{&out};
    std::cout << "  协程: 已恢复\n";
    co_return;
}

inline AsyncTask demo_thread(int delay_ms) {
    std::cout << "  协程: co_await ThreadAwaitable{" << delay_ms << "ms}\n";
    co_await ThreadAwaitable{delay_ms};
    std::cout << "  协程: 已恢复，线程 id = " << std::this_thread::get_id() << "\n";
    co_return;
}

// ───────────────────────────────────────────────────────────────────────────
inline void run_module4() {
    std::cout << "\n╔══ M4 co_await 与 Awaitable（策略模式）══════╗\n";

    // ── 策略 1: ReadyAwaitable ──────────────────────────────────────────────
    std::cout << "\n── Demo 1: ReadyAwaitable（await_ready=true，不挂起）──\n";
    {
        auto t = demo_ready();
        t.resume();  // 一次 resume 跑完全程
    }

    // ── 策略 2a: ConditionalAwaitable（条件满足，不挂起）───────────────────
    std::cout << "\n── Demo 2a: ConditionalAwaitable（条件满足 → 不挂起）──\n";
    {
        auto t = demo_conditional(true);
        t.resume();  // 一次 resume 跑完
    }

    // ── 策略 2b: ConditionalAwaitable（条件不满足，真挂起）─────────────────
    std::cout << "\n── Demo 2b: ConditionalAwaitable（条件不满足 → 挂起）──\n";
    {
        auto t = demo_conditional(false);
        t.resume();  // 启动 → 在 await_suspend 返回 true → 挂起
        std::cout << "  主线程: 协程已挂起，手动再次 resume\n";
        t.resume();  // 恢复
    }

    // ── 策略 3: ManualAwaitable ─────────────────────────────────────────────
    std::cout << "\n── Demo 3: ManualAwaitable（外部持有句柄）──\n";
    {
        std::coroutine_handle<> saved;
        auto t = demo_manual(saved);
        t.resume();  // 启动 → 协程挂起，saved 被写入
        std::cout << "  主线程: 协程已挂起，模拟异步回调，调用 saved.resume()\n";
        saved.resume();  // 外部触发恢复
    }

    // ── 策略 4: ThreadAwaitable（真实跨线程 resume）────────────────────────
    std::cout << "\n── Demo 4: ThreadAwaitable（50ms 后后台线程 resume）──\n";
    {
        std::promise<void> done;
        auto future = done.get_future();

        auto t = demo_thread(50);
        t.handle.promise().completion = &done;

        std::cout << "  主线程 id = " << std::this_thread::get_id() << "\n";
        t.handle.resume();  // 启动协程，在 ThreadAwaitable 挂起，后台线程 detach

        std::cout << "  主线程: 等待后台线程完成...\n";
        future.wait();
        std::cout << "  主线程: 已收到完成通知\n";
    }

    std::cout << "╚═════════════════════════════════════════════╝\n";
}

} // namespace m4
