# 模块 4：co_await 与 Awaitable

---

## 设计模式：策略模式 (Strategy Pattern)

Awaitable 是策略模式的协程实现：
- **Context**：协程（执行 `co_await expr`）
- **Strategy 接口**：Awaitable 协议（`await_ready/suspend/resume` 三个方法）
- **具体策略**：`suspend_never`、`suspend_always`、自定义 Awaitable

不同的 Awaitable 策略决定了协程在等待时的行为：
- 立即继续（不挂起）
- 无条件挂起
- 条件挂起（取决于操作是否已完成）
- 挂起并将句柄交给外部（调度器、IO 事件等）

---

## Awaitable 协议（三个方法）

```cpp
struct MyAwaitable {
    // 1. 是否需要挂起？
    //    true  → 不挂起，直接调用 await_resume()
    //    false → 挂起，调用 await_suspend()
    bool await_ready();

    // 2. 挂起时的行为（三种返回类型，见下）
    ??? await_suspend(std::coroutine_handle<> caller);

    // 3. 协程恢复时，co_await 表达式的求值结果
    T await_resume();
};
```

---

## await_suspend 的三种返回类型

### 返回 void：无条件挂起

```cpp
void await_suspend(std::coroutine_handle<> h) {
    // 将 h 交给某个调度器，调度器负责在适当时机调用 h.resume()
    scheduler.post(h);
}
```

### 返回 bool：条件挂起

```cpp
bool await_suspend(std::coroutine_handle<> h) {
    // true  → 挂起（控制权返回调用者）
    // false → 不挂起（立即继续执行协程）
    if (result_ready) {
        return false;  // 结果已就绪，无需挂起
    }
    save_continuation(h);
    return true;
}
```

### 返回 coroutine_handle<>：对称传输

```cpp
std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) {
    save_continuation(h);
    // 直接跳转到另一个协程，不经过调用者（避免栈溢出）
    return next_coroutine;
}
```

---

## co_await 完整执行流程

```
co_await expr

1. 获取 Awaitable 对象（expr 本身，或 promise.await_transform(expr)）
2. 调用 awaitable.await_ready()
   │
   ├── true  → 跳到步骤 5
   │
   └── false → 挂起当前协程
               调用 awaitable.await_suspend(current_handle)
               │
               ├── void 返回：控制权回到调用者
               ├── bool true：控制权回到调用者
               ├── bool false：恢复当前协程（不真正挂起）
               └── handle：跳转到指定协程（对称传输）
               │
               ... 某时刻 current_handle.resume() 被调用 ...
               │
5. 调用 awaitable.await_resume()
   返回值即 co_await expr 的值
```

---

## 内置 Awaitable

```cpp
// 永远不挂起
std::suspend_never {
    bool await_ready() { return true; }
    void await_suspend(std::coroutine_handle<>) {}
    void await_resume() {}
};

// 永远挂起
std::suspend_always {
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<>) {}
    void await_resume() {}
};
```

---

## await_transform：拦截 co_await

`promise_type` 可以定义 `await_transform`，在协程执行 `co_await expr` 时拦截：

```cpp
struct promise_type {
    // 将 int 转换成一个等待 N 毫秒的 Awaitable
    auto await_transform(int milliseconds) {
        return TimerAwaitable{milliseconds};
    }

    // 将协程句柄转换为可等待对象（Task<T> 的关键）
    template<typename T>
    auto await_transform(Task<T>& task) {
        return task.operator co_await();
    }
};

// 使用时：
Task<void> example() {
    co_await 100;     // 等待 100ms（由 await_transform 处理）
}
```

---

## 示例场景（见 module4_awaitable/）

1. **`ReadyAwaitable`** — 立即完成（await_ready = true）
2. **`ManualAwaitable`** — 手动控制 resume 时机
3. **`SwitchAwaitable`** — 切换执行上下文（策略：转移句柄给另一个线程）
4. **`TimerAwaitable`** — 模拟定时挂起

---

## 关键规则

| 规则 | 说明 |
|------|------|
| `await_ready` 返回 `true` | 跳过挂起，直接调用 `await_resume` |
| 对称传输 | `await_suspend` 返回 handle，编译器做 tail-call，无栈增长 |
| `noop_coroutine()` | 特殊句柄，resume 是空操作，用于"无事可做"的终止 |
| `promise.await_transform` | 可用于禁止某些 `co_await`（`= delete`） |

---

**下一步** → [模块5：异步任务 Task<T>](05_task.md)
