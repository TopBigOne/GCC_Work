# 模块 2：Promise 类型

---

## 设计模式：模板方法模式 (Template Method Pattern)

`promise_type` 是 C++20 协程框架对**模板方法模式**的典型运用：

- 框架（编译器）定义了协程的执行算法骨架（生命周期）
- `promise_type` 的各个方法是算法中的"钩子"（hook）
- 用户通过实现这些钩子，定制协程的具体行为

```
协程生命周期算法（框架定义，不可修改）：
  1. 构造 promise_type
  2. 调用 get_return_object()          ← 钩子
  3. co_await initial_suspend()        ← 钩子
  4. 执行协程体
     - 遇到 co_yield v → yield_value(v) ← 钩子
     - 遇到 co_return v → return_value(v)← 钩子
     - 遇到异常 → unhandled_exception() ← 钩子
  5. co_await final_suspend()          ← 钩子
  6. 销毁协程帧
```

---

## promise_type 必须实现的接口

```cpp
struct promise_type {
    // 1. 构建并返回协程对应的 ReturnObject
    ReturnObject get_return_object();

    // 2. 协程启动时是否立即挂起
    //    返回 suspend_never  → 立即执行
    //    返回 suspend_always → 惰性启动，等待第一次 resume
    auto initial_suspend();

    // 3. 协程结束时是否挂起（必须 noexcept）
    //    返回 suspend_never  → 自动销毁帧
    //    返回 suspend_always → 调用者手动 destroy()
    auto final_suspend() noexcept;

    // 4a. 协程以 co_return; 结束（void 协程）
    void return_void();

    // 4b. 协程以 co_return value; 结束（有返回值）
    //     注意：return_void 和 return_value 二选一
    void return_value(T value);

    // 5. 协程体内发生异常时调用
    void unhandled_exception();
};
```

---

## initial_suspend 与 final_suspend 的选择

```
initial_suspend:
  suspend_never  → 调用协程函数时，协程立即开始运行（eager）
  suspend_always → 调用协程函数时，协程不运行，返回对象后
                   需要手动 handle.resume() 才开始（lazy）

final_suspend:
  suspend_never  → 协程结束后帧立即销毁，handle 失效
                   简单场景使用，但无法在结束后读取结果
  suspend_always → 协程结束后帧暂停，调用者可读取结果后
                   手动 handle.destroy() 销毁
```

---

## 对称传输 (Symmetric Transfer)

`final_suspend` 返回一个自定义 Awaiter，可在协程结束时**直接跳转到另一个协程**，避免栈溢出：

```cpp
struct FinalAwaiter {
    bool await_ready() noexcept { return false; }

    // 返回 continuation（下一个要执行的协程）
    // 编译器会做 tail-call 优化，不增加栈深度
    std::coroutine_handle<> await_suspend(
        std::coroutine_handle<promise_type> h) noexcept {
        if (h.promise().continuation)
            return h.promise().continuation;
        return std::noop_coroutine();  // 什么都不做
    }

    void await_resume() noexcept {}
};
```

---

## 完整示例结构

见 `module2_promise/main.cpp`，包含三个递进示例：

1. **`FireAndForget`** — 最简 void 协程，`suspend_never`，运行完自动销毁
2. **`LazyTask`** — 惰性启动，`initial_suspend` 返回 `suspend_always`
3. **`ValueTask<T>`** — 有返回值，`final_suspend` 暂停以便读取结果

---

## 编译器对协程函数的变换

编译器将协程函数变换为大致等价的代码：

```cpp
// 原始协程
Task my_coro() {
    co_return 42;
}

// 编译器内部（伪代码）
Task my_coro() {
    // 1. 在堆上分配协程帧
    auto* frame = operator new(sizeof(CoroutineFrame));

    // 2. 在帧中构造 promise
    auto& p = frame->promise;

    // 3. 获取返回对象
    Task ret = p.get_return_object();

    // 4. 执行 initial_suspend
    co_await p.initial_suspend();

    // 5. 执行协程体（包含各种挂起点）
    try {
        /* 协程体 */
        p.return_value(42);
    } catch (...) {
        p.unhandled_exception();
    }

    // 6. 执行 final_suspend
    co_await p.final_suspend();

    // 7. 销毁帧（如果 final_suspend 返回 suspend_never）
    operator delete(frame);

    return ret;
}
```

---

## 关键点总结

| 方法 | 何时调用 | 返回类型 |
|------|----------|----------|
| `get_return_object` | 协程初始化时，最先调用 | `ReturnObject`（自定义） |
| `initial_suspend` | 协程体执行前 | Awaitable |
| `return_void/value` | 遇到 `co_return` | `void` |
| `unhandled_exception` | 协程体抛出异常 | `void` |
| `final_suspend` | 协程结束后 | Awaitable（noexcept） |

---

**下一步** → [模块3：co_yield 与生成器](03_generator.md)
