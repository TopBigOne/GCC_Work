#pragma once
#include "scheduler.h"
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// 观察者模式 (Observer Pattern)：Event
//
//   Subject  = Event（被等待的事件）
//   Observer = 等待该事件的各协程（通过 co_await event 注册）
//
// 工作流程：
//   1. 协程 co_await event → 注册自己的 handle 到 event.waiters_
//   2. 其他代码调用 event.trigger() → 把所有等待者重新投入 Scheduler
//   3. Scheduler::run() 恢复这些协程
//
// 特性：
//   • 支持多个协程等待同一 Event
//   • 触发前已设置 triggered_ = true，后来者 await_ready() 直接通过，不挂起
// ─────────────────────────────────────────────────────────────────────────────

namespace m6 {

class Event {
public:
    // ── Awaiter：等待 Event 触发 ─────────────────────────────────────────
    struct Awaiter {
        Event&     event_;
        Scheduler& sched_;

        bool await_ready() noexcept {
            return event_.triggered_;  // 已触发则直接通过，不挂起
        }

        void await_suspend(std::coroutine_handle<> h) noexcept {
            event_.waiters_.push_back(h);
        }

        void await_resume() noexcept {}
    };

    // co_await event 语法糖（需传入 Scheduler 以便 trigger 时复原）
    Awaiter wait(Scheduler& s) noexcept { return Awaiter{*this, s}; }

    // 触发事件：唤醒所有等待者
    void trigger(Scheduler& s) {
        triggered_ = true;
        for (auto h : waiters_) s.post(h);
        waiters_.clear();
    }

    void reset() noexcept { triggered_ = false; }
    bool triggered() const noexcept { return triggered_; }

private:
    bool                                  triggered_ = false;
    std::vector<std::coroutine_handle<>>  waiters_;
};

} // namespace m6
