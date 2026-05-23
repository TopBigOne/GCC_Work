# 模块 6：实战应用

---

## 设计模式：Reactor 模式 + 观察者模式

### Reactor 模式（协程调度器）

```
┌──────────────────────────────────┐
│           Scheduler              │
│  ┌────────────────────────────┐  │
│  │    Ready Queue             │  │  ← 就绪的协程句柄
│  │  [handle1, handle2, ...]   │  │
│  └────────────────────────────┘  │
│                                  │
│  run() {                         │
│    while (!queue.empty()) {      │
│      handle = queue.pop();       │
│      handle.resume();            │  ← 驱动协程
│    }                             │
│  }                               │
└──────────────────────────────────┘
         ↑  post(handle)
         │
   协程挂起时，把自己的 handle 提交给调度器
```

### 观察者模式（Task 完成通知）

- **Subject**：`Task<T>`（被等待的协程）
- **Observer**：等待者协程（continuation）
- Task 完成时，通过 `final_suspend` 对称传输自动通知观察者

---

## 简单协程调度器

见 `module6_realworld/scheduler.h`：

```cpp
class Scheduler {
public:
    // 提交一个就绪的协程
    void post(std::coroutine_handle<> handle) {
        ready_queue_.push(handle);
    }

    // 运行直到所有协程完成
    void run() {
        while (!ready_queue_.empty()) {
            auto handle = ready_queue_.front();
            ready_queue_.pop();
            handle.resume();
        }
    }

private:
    std::queue<std::coroutine_handle<>> ready_queue_;
};

// 全局调度器（简化版）
inline Scheduler& get_scheduler() {
    static Scheduler s;
    return s;
}
```

---

## 可调度的 Awaitable

```cpp
// 一个将协程提交给调度器的 Awaitable
struct ScheduledAwaitable {
    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        // 不立即 resume，而是把 handle 交给调度器
        get_scheduler().post(h);
    }

    void await_resume() {}
};

// 使用
Task<void> work() {
    std::cout << "开始\n";
    co_await ScheduledAwaitable{};  // 让出控制权，允许其他协程运行
    std::cout << "继续\n";
}
```

---

## 线程池集成

```cpp
class ThreadPoolAwaitable {
    std::thread::id target_thread_;
public:
    explicit ThreadPoolAwaitable(ThreadPool& pool) : pool_(pool) {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        // 把 resume 动作提交到线程池
        pool_.submit([h]() mutable { h.resume(); });
    }

    void await_resume() {}
private:
    ThreadPool& pool_;
};

// 使用
Task<void> cpu_task() {
    co_await switch_to_thread_pool(pool);  // 切换到线程池线程
    // 现在在线程池线程上运行
    do_heavy_computation();
    co_await switch_to_main_thread();      // 切换回主线程
    update_ui();
}
```

---

## 实战 Demo（module6_realworld/main.cpp）

### Demo 1：多协程并发调度

```
Scheduler 同时调度 3 个协程，交替执行，演示协作式多任务：

Task A: step1 → yield → step2 → yield → step3
Task B: step1 → yield → step2 → yield → step3
Task C: step1 → yield → step2 → yield → step3

输出顺序：A1 B1 C1 A2 B2 C2 A3 B3 C3
（而不是 A1 A2 A3 B1 B2 B3...）
```

### Demo 2：观察者模式 - 事件系统

```cpp
// Event：可被多个协程等待，触发时通知所有等待者
class Event {
    std::vector<std::coroutine_handle<>> waiters_;
    bool triggered_ = false;
public:
    // Awaitable：等待事件
    auto operator co_await() {
        return EventAwaiter{*this};
    }

    // 触发：唤醒所有等待者
    void trigger() {
        triggered_ = true;
        for (auto h : waiters_) get_scheduler().post(h);
        waiters_.clear();
    }
};
```

### Demo 3：生产者-消费者

```cpp
// Channel<T>：协程安全的消息通道
// 生产者 co_await channel.send(value)
// 消费者 T value = co_await channel.receive()
```

---

## 常见陷阱

### 1. 悬空引用

```cpp
// 危险！局部变量在挂起后可能已被销毁
Task<void> bad() {
    std::string local = "hello";
    co_await some_async_op();
    // 如果 local 在挂起期间被栈帧销毁 → 未定义行为
    // 但实际上协程帧在堆上，local 是协程帧的一部分，是安全的
    // 真正的危险：对外部栈变量的引用
    use(local);  // OK
}

Task<void> real_danger(std::string& ref) {
    co_await some_async_op();
    use(ref);  // 危险！ref 指向的对象可能已析构
}
```

### 2. 协程帧大小

- 协程帧存储所有跨挂起点存活的局部变量
- 避免在协程中持有大型对象，用 `unique_ptr` 包装
- 使用 `-fcoro-aligned-allocation` 或自定义 `operator new`

### 3. 不能在析构函数中 co_await

```cpp
struct Bad {
    ~Bad() {
        co_await something();  // 编译错误：析构函数不能是协程
    }
};
```

---

## 生产环境建议

| 场景 | 推荐方案 |
|------|----------|
| 异步网络IO | Boost.Asio + 协程（`asio::awaitable<T>`） |
| 高性能协程 | cppcoro 库（Lewis Baker） |
| 嵌入式/低延迟 | 手写最小 Promise，避免动态分配 |
| 取消支持 | 使用 `stop_token` + `stop_source`（C++20） |

---

**恭喜！** 完成全部 6 个模块后，你已掌握 C++20 协程的核心机制。
