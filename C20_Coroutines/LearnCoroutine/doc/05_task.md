# 模块 5：异步任务 Task<T>

---

## 设计模式：责任链模式 (Chain of Responsibility)

`Task<T>` 的 continuation 机制是责任链模式的体现：

- 每个 Task 持有"完成后继续谁"（continuation）
- 当一个 Task 完成时，自动将控制权传递给链中下一个 Task
- 链式 `co_await` 构成一条责任链，无需手动管理谁调用谁

```
TaskA co_awaits TaskB
TaskB co_awaits TaskC

执行流：
  resume(TaskA)
    → TaskA 挂起，把自己作为 continuation 交给 TaskB
    → resume(TaskB)
      → TaskB 挂起，把自己作为 continuation 交给 TaskC
      → resume(TaskC)
        → TaskC 完成 → final_suspend
          → 恢复 TaskB（对称传输）
        → TaskB 完成 → final_suspend
          → 恢复 TaskA（对称传输）
      → TaskA 完成
```

---

## 为什么需要 Task<T>

```
FireAndForget（模块2）的局限：
  ✗ 无法等待另一个协程完成再继续
  ✗ 无法获取协程的返回值
  ✗ 异常会丢失

Task<T> 解决了这些问题：
  ✓ co_await task → 挂起当前协程，等待 task 完成
  ✓ co_await task → 求值为 task 的返回值 T
  ✓ 异常会从 task 传播给等待者
```

---

## Task<T> 核心组件

### 1. Promise：存储结果 + 管理 continuation

```cpp
struct promise_type {
    // 用 variant 存储三种状态：未完成 / 有结果 / 有异常
    std::variant<std::monostate, T, std::exception_ptr> result_;

    // 谁在等我？（责任链中的上一个 Task）
    std::coroutine_handle<> continuation_;

    // 关键：final_suspend 做对称传输
    struct FinalAwaiter {
        bool await_ready() noexcept { return false; }
        std::coroutine_handle<> await_suspend(
            std::coroutine_handle<promise_type> h) noexcept {
            // 如果有人在等我，把控制权直接给它（对称传输）
            auto cont = h.promise().continuation_;
            return cont ? cont : std::noop_coroutine();
        }
        void await_resume() noexcept {}
    };

    FinalAwaiter final_suspend() noexcept { return {}; }
};
```

### 2. Awaiter：实现 co_await task 的行为

```cpp
struct Awaiter {
    std::coroutine_handle<promise_type> handle_;

    // 如果 Task 已经完成，不需要挂起
    bool await_ready() { return handle_.done(); }

    // 挂起当前协程，把自己作为 continuation 注册到 task
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller) {
        handle_.promise().continuation_ = caller;
        return handle_;  // 对称传输：直接切换到 task
    }

    // task 完成后，取回结果（或重新抛出异常）
    T await_resume() {
        auto& result = handle_.promise().result_;
        if (auto* ex = std::get_if<std::exception_ptr>(&result))
            std::rethrow_exception(*ex);
        return std::get<T>(result);
    }
};
```

---

## 完整使用示例

```cpp
Task<int> fetch_data(int id) {
    co_return id * 10;  // 模拟异步获取
}

Task<std::string> process(int id) {
    int data = co_await fetch_data(id);   // 等待并获取结果
    co_return "result: " + std::to_string(data);
}

Task<void> main_task() {
    auto r1 = co_await process(1);  // "result: 10"
    auto r2 = co_await process(2);  // "result: 20"
    std::cout << r1 << "\n" << r2 << "\n";
}
```

---

## 异常传播

```cpp
Task<int> may_fail(bool fail) {
    if (fail) throw std::runtime_error("出错了");
    co_return 42;
}

Task<void> caller() {
    try {
        int v = co_await may_fail(true);  // 异常从这里传出
    } catch (const std::exception& e) {
        std::cout << "捕获: " << e.what() << "\n";
    }
}
```

异常传播路径：
1. `may_fail` 抛出异常
2. `promise_type::unhandled_exception()` 捕获并存储
3. `final_suspend` 对称传输到 `caller`
4. `await_resume()` 重新抛出异常
5. `caller` 的 try-catch 捕获

---

## Task<void> 特化

`co_return;` 对应 `return_void()`，需要特化处理：

```cpp
template<>
struct Task<void>::promise_type {
    void return_void() {}  // 而不是 return_value
    // 其余相同...
};
```

---

## 关键设计决策

| 决策 | 选择 | 原因 |
|------|------|------|
| `initial_suspend` | `suspend_always` | 惰性：创建 Task 不立即执行，由调用者控制 |
| `final_suspend` | 自定义 FinalAwaiter | 对称传输恢复等待者，避免栈溢出 |
| 结果存储 | `std::variant` | 类型安全地存储值或异常 |
| 所有权 | `unique_ptr` 语义 | Task 独占 coroutine_handle，析构时 destroy |

---

## 实现在 module5_task/

- `task.h` — Task<T> 完整实现（含 void 特化）
- `main.cpp` — 责任链演示：多级 co_await 链式调用 + 异常传播

---

**下一步** → [模块6：实战应用](06_realworld.md)
