#pragma once
#include "scheduler.h"
#include <queue>

// ─────────────────────────────────────────────────────────────────────────────
// 生产者-消费者模式：Channel<T>
//
// 设计：
//   • send(v)     ：同步投递。若有等待的接收者，直接交付并唤醒；否则入缓冲区。
//   • receive(s)  ：返回 Awaiter。若缓冲区有数据，直接取出（不挂起）；
//                   否则挂起，等待 send() 交付数据后被 Scheduler 唤醒。
//
// 单线程（Scheduler 驱动）：不需要互斥锁，无竞态。
// ─────────────────────────────────────────────────────────────────────────────

namespace m6 {

template<typename T>
class Channel {
public:
    // ── RecvAwaiter：co_await channel.receive(sched) 的行为 ──────────────
    struct RecvAwaiter {
        Channel&   ch_;
        Scheduler& sched_;
        T          result_{};  // 最终交付的值

        bool await_ready() noexcept {
            if (!ch_.buffer_.empty()) {
                result_ = std::move(ch_.buffer_.front());
                ch_.buffer_.pop();
                return true;  // 有数据，直接取，不挂起
            }
            return false;
        }

        bool await_suspend(std::coroutine_handle<> h) noexcept {
            // 二次检查（await_ready 已处理一般情况，这里是 await_suspend 被调用的情况）
            if (!ch_.buffer_.empty()) {
                result_ = std::move(ch_.buffer_.front());
                ch_.buffer_.pop();
                return false;  // 不挂起
            }
            // 记录等待者
            ch_.waiters_.push({h, this});
            return true;  // 挂起，等待 send 唤醒
        }

        T await_resume() noexcept { return std::move(result_); }
    };

    // ── send：投递数据 ────────────────────────────────────────────────────
    void send(T value, Scheduler& s) {
        if (!waiters_.empty()) {
            // 直接交付给最早等待的接收者
            auto [h, awaiter] = waiters_.front();
            waiters_.pop();
            awaiter->result_ = std::move(value);
            s.post(h);  // 唤醒接收方
        } else {
            buffer_.push(std::move(value));
        }
    }

    RecvAwaiter receive(Scheduler& s) { return RecvAwaiter{*this, s}; }

    bool empty() const noexcept { return buffer_.empty(); }

private:
    std::queue<T>                                            buffer_;
    std::queue<std::pair<std::coroutine_handle<>, RecvAwaiter*>> waiters_;
};

} // namespace m6
