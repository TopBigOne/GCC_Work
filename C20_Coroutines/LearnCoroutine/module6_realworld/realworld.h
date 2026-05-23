#pragma once
#include "scheduler.h"
#include "event.h"
#include "channel.h"
#include <iostream>
#include <string>

namespace m6 {

// ═══════════════════════════════════════════════════════════════════════════
// Demo 1: 协作式多任务调度（Reactor 模式）
//
// 3 个协程各执行 3 步，每步后 co_await Yield{sched} 让出。
// Scheduler 按 FIFO 顺序驱动，输出交叉顺序：A1 B1 C1 A2 B2 C2 A3 B3 C3
// ═══════════════════════════════════════════════════════════════════════════
inline SchedTask worker(Scheduler& sched, char name, int steps) {
    for (int i = 1; i <= steps; ++i) {
        std::cout << "  [Worker " << name << "] step " << i << "\n";
        co_await Yield{sched};  // 让出，其他协程获得运行机会
    }
    std::cout << "  [Worker " << name << "] 完成\n";
    co_return;
}

inline void demo_scheduler() {
    std::cout << "\n── Demo 1: 协作式调度（Reactor 模式）──\n";
    Scheduler sched;

    auto a = worker(sched, 'A', 3);
    auto b = worker(sched, 'B', 3);
    auto c = worker(sched, 'C', 3);

    a.schedule(sched);
    b.schedule(sched);
    c.schedule(sched);

    std::cout << "  [Scheduler] 开始 run()\n";
    sched.run();
    std::cout << "  [Scheduler] run() 结束，队列已空\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// Demo 2: Event（观察者模式）
//
// 多个协程 co_await event.wait(sched)，等待同一事件。
// trigger() 时批量唤醒所有等待者。
// ═══════════════════════════════════════════════════════════════════════════
inline SchedTask event_waiter(Scheduler& sched, Event& evt, std::string name) {
    std::cout << "  [" << name << "] 等待事件\n";
    co_await evt.wait(sched);
    std::cout << "  [" << name << "] 事件触发，继续执行\n";
    co_return;
}

inline SchedTask event_trigger(Scheduler& sched, Event& evt) {
    // 先让出，让所有 waiter 协程都注册到 event
    co_await Yield{sched};
    co_await Yield{sched};
    co_await Yield{sched};
    std::cout << "  [触发者] 触发事件\n";
    evt.trigger(sched);
    co_return;
}

inline void demo_event() {
    std::cout << "\n── Demo 2: Event（观察者模式）──\n";
    Scheduler sched;
    Event     evt;

    auto w1 = event_waiter(sched, evt, "Waiter-1");
    auto w2 = event_waiter(sched, evt, "Waiter-2");
    auto w3 = event_waiter(sched, evt, "Waiter-3");
    auto tr = event_trigger(sched, evt);

    w1.schedule(sched);
    w2.schedule(sched);
    w3.schedule(sched);
    tr.schedule(sched);

    sched.run();
}

// ═══════════════════════════════════════════════════════════════════════════
// Demo 3: Channel<T>（生产者-消费者模式）
//
// 生产者 send 数据；消费者 co_await channel.receive(sched)。
// 若无数据，消费者挂起；send 时直接交付并唤醒。
// ═══════════════════════════════════════════════════════════════════════════
inline SchedTask producer(Scheduler& sched, Channel<int>& ch) {
    for (int i = 1; i <= 3; ++i) {
        std::cout << "  [Producer] 发送 " << i << "\n";
        ch.send(i, sched);
        co_await Yield{sched};  // 让出，消费者有机会运行
    }
    co_return;
}

inline SchedTask consumer(Scheduler& sched, Channel<int>& ch, std::string name) {
    for (int i = 0; i < 3; ++i) {
        int val = co_await ch.receive(sched);
        std::cout << "  [" << name << "] 收到 " << val << "\n";
        co_await Yield{sched};
    }
    co_return;
}

inline void demo_channel() {
    std::cout << "\n── Demo 3: Channel<int>（生产者-消费者）──\n";
    Scheduler   sched;
    Channel<int> ch;

    auto prod = producer(sched, ch);
    auto cons = consumer(sched, ch, "Consumer");

    prod.schedule(sched);
    cons.schedule(sched);

    sched.run();
}

// ───────────────────────────────────────────────────────────────────────────
inline void run_module6() {
    std::cout << "\n╔══ M6 实战应用（Reactor + 观察者模式）═══════╗\n";

    demo_scheduler();
    demo_event();
    demo_channel();

    std::cout << "╚═════════════════════════════════════════════╝\n";
}

} // namespace m6
