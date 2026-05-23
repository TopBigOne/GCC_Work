#pragma once
#include <coroutine>
#include <iostream>

namespace m1 {

// ═══════════════════════════════════════════════════════════════════════════
// Demo 1: SimpleTask
// initial_suspend = suspend_never  → 调用时立即执行
// final_suspend   = suspend_never  → 结束后自动销毁帧
// 调用者拿到 SimpleTask 时，协程已经跑完了。
// ═══════════════════════════════════════════════════════════════════════════
struct SimpleTask {
    struct promise_type {
        SimpleTask get_return_object() noexcept { return {}; }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };
};

inline SimpleTask simple_coro() {
    std::cout << "  [SimpleTask] 协程开始执行\n";
    // 无任何挂起点，直接跑完
    std::cout << "  [SimpleTask] 协程结束\n";
    co_return;
}

// ═══════════════════════════════════════════════════════════════════════════
// Demo 2: ManualTask
// initial_suspend = suspend_always → 创建后不立即执行，需手动 resume()
// final_suspend   = suspend_never  → 结束后自动销毁
// 演示：调用者完全控制协程何时开始运行。
// ═══════════════════════════════════════════════════════════════════════════
struct ManualTask {
    struct promise_type {
        ManualTask get_return_object() noexcept {
            return ManualTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        // suspend_always：帧在协程结束后由 ~ManualTask() 安全销毁
        // 若改成 suspend_never，runtime 会立即 free 帧，
        // 此后析构里再碰 handle 就是野指针（SIGBUS）
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit ManualTask(std::coroutine_handle<promise_type> h) noexcept : handle(h) {}
    ManualTask(const ManualTask&) = delete;
    ManualTask(ManualTask&& o) noexcept : handle(o.handle) { o.handle = nullptr; }
    ~ManualTask() {
        // final_suspend = suspend_always 保证帧一直有效，可以安全 destroy
        if (handle) handle.destroy();
    }
};

inline ManualTask manual_coro() {
    std::cout << "  [ManualTask] Step A — 第一次 resume 后执行到这里\n";
    co_await std::suspend_always{};  // 第一个挂起点
    std::cout << "  [ManualTask] Step B — 第二次 resume 后执行到这里\n";
    co_return;
}

// ═══════════════════════════════════════════════════════════════════════════
// Demo 3: StepTask
// initial_suspend = suspend_always → 惰性启动
// final_suspend   = suspend_always → 结束后帧不销毁，让调用者检查 done()
// 演示：多次 co_await 分步执行，逐步推进协程。
// ═══════════════════════════════════════════════════════════════════════════
struct StepTask {
    struct promise_type {
        StepTask get_return_object() noexcept {
            return StepTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit StepTask(std::coroutine_handle<promise_type> h) noexcept : handle(h) {}
    StepTask(const StepTask&) = delete;
    StepTask(StepTask&& o) noexcept : handle(o.handle) { o.handle = nullptr; }
    ~StepTask() { if (handle) handle.destroy(); }

    bool done() const noexcept { return !handle || handle.done(); }
    void resume() { if (!done()) handle.resume(); }
};

inline StepTask step_coro() {
    std::cout << "  [StepTask] Step 1 — 即将第一次挂起\n";
    co_await std::suspend_always{};
    std::cout << "  [StepTask] Step 2 — 恢复后再次挂起\n";
    co_await std::suspend_always{};
    std::cout << "  [StepTask] Step 3 — 最后一步，即将结束\n";
    co_return;
}

// ───────────────────────────────────────────────────────────────────────────
inline void run_module1() {
    std::cout << "\n╔══ M1 协程基础概念 ══════════════════════════╗\n";

    std::cout << "\n── Demo 1: SimpleTask（suspend_never，立即执行）──\n";
    simple_coro();  // 调用即完成

    std::cout << "\n── Demo 2: ManualTask（suspend_always，手动 resume）──\n";
    {
        auto t = manual_coro();
        std::cout << "  主线程: 协程已创建，尚未执行\n";
        t.handle.resume();  // 启动，跑到第一个 co_await 挂起
        std::cout << "  主线程: 第一次 resume 返回，协程挂起中\n";
        t.handle.resume();  // 恢复，跑到结束
        std::cout << "  主线程: 第二次 resume 返回，协程已完成\n";
    }

    std::cout << "\n── Demo 3: StepTask（多步暂停/恢复）──\n";
    {
        auto t = step_coro();
        for (int step = 1; !t.done(); ++step) {
            std::cout << "  主线程: 第 " << step << " 次 resume\n";
            t.resume();
        }
        std::cout << "  主线程: 协程已完成\n";
    }

    std::cout << "╚═════════════════════════════════════════════╝\n";
}

} // namespace m1
