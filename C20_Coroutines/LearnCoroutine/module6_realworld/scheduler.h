#pragma once
#include <coroutine>
#include <queue>

// ─────────────────────────────────────────────────────────────────────────────
// Reactor 模式：Scheduler + SchedTask
//
//   Scheduler  ←  ready queue（就绪协程句柄的 FIFO 队列）
//   run()      →  依次 resume 队列中的协程（事件循环）
//   SchedTask  →  initial_suspend=always，等待 schedule(sched) 投入队列
//   Yield      →  协程主动让出，把自己重新投回队列
//
// 协作式调度：协程主动 co_await Yield{sched} 让出控制权，
//             Scheduler 在下一轮按顺序驱动其他协程。
// ─────────────────────────────────────────────────────────────────────────────

namespace m6 {

// ── Scheduler：就绪队列 + 驱动循环 ──────────────────────────────────────────
class Scheduler {
public:
    void post(std::coroutine_handle<> h) { ready_.push(h); }

    // 驱动所有就绪协程直到队列为空
    void run() {
        while (!ready_.empty()) {
            auto h = ready_.front();
            ready_.pop();
            h.resume();
        }
    }

    bool empty() const noexcept { return ready_.empty(); }

private:
    std::queue<std::coroutine_handle<>> ready_;
};

// ── Yield：协程主动让出，把自己重新投入调度队列 ──────────────────────────
struct Yield {
    Scheduler& sched;

    bool await_ready()  noexcept { return false; }
    void await_suspend(std::coroutine_handle<> h) noexcept { sched.post(h); }
    void await_resume() noexcept {}
};

// ── SchedTask：可被 Scheduler 调度的协程任务 ─────────────────────────────
// initial_suspend = suspend_always：创建时不运行，等待 schedule() 投入队列
// final_suspend   = suspend_always：协程完成后帧挂起，由 ~SchedTask() 安全销毁
//
// ⚠️  不能用 suspend_never：sched.run() 返回后析构检查 handle.done() 是野指针
class SchedTask {
public:
    struct promise_type {
        SchedTask get_return_object() noexcept {
            return SchedTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend()   noexcept { return {}; }
        void return_void()         noexcept {}
        void unhandled_exception() noexcept { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit SchedTask(std::coroutine_handle<promise_type> h) noexcept : handle(h) {}
    SchedTask(const SchedTask&) = delete;
    SchedTask(SchedTask&& o) noexcept : handle(o.handle) { o.handle = nullptr; }
    // handle 始终有效（suspend_always 保证帧未被 runtime 提前销毁）
    ~SchedTask() { if (handle) handle.destroy(); }

    // 投入调度队列（必须在 sched.run() 前调用）
    void schedule(Scheduler& s) { s.post(handle); }
};

} // namespace m6
