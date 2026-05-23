# 模块 1：协程基础概念

---

## 1. 什么是协程

协程（Coroutine）是一种**可以暂停自身执行并在之后恢复**的函数。

| 对比维度 | 普通函数 | 协程 | 线程 |
|----------|----------|------|------|
| 执行模型 | 调用即运行到结束 | 可多次暂停/恢复 | 并发执行 |
| 调用栈 | 有 | 无（帧存堆上） | 有（独立栈） |
| 切换开销 | 无 | 极低（用户态） | 高（内核态） |
| 内存 | 栈上 | 堆上（协程帧） | 独立栈（默认 8MB） |
| 适合场景 | 普通逻辑 | 异步IO、生成器 | CPU密集并行 |

---

## 2. C++20 协程的三个关键字

一个函数体中只要出现以下任一关键字，该函数就是协程：

```cpp
co_await expr    // 挂起，等待 expr 完成
co_yield expr    // 产出一个值，然后挂起
co_return expr   // 结束协程，返回一个值（或 co_return; 返回 void）
```

---

## 3. 协程整体架构

```
调用者 (caller)
    │
    │ 调用协程函数
    ▼
┌─────────────────────────────┐
│        协程函数              │
│  ┌──────────────────────┐   │
│  │    promise_type      │   │  ← 控制协程行为的策略对象
│  │  - get_return_object │   │
│  │  - initial_suspend   │   │
│  │  - final_suspend     │   │
│  │  - yield_value       │   │
│  │  - return_value      │   │
│  └──────────────────────┘   │
│                             │
│  ┌──────────────────────┐   │
│  │   coroutine_handle   │   │  ← 协程的"遥控器"
│  │  - resume()          │   │
│  │  - destroy()         │   │
│  │  - done()            │   │
│  └──────────────────────┘   │
│                             │
│  协程帧（Heap）              │  ← 保存局部变量、挂起点
└─────────────────────────────┘
          │
          │ 返回 ReturnObject（Task/Generator 等）
          ▼
     调用者持有 ReturnObject
```

---

## 4. 协程生命周期

```
协程被调用
    │
    ▼
promise_type 构造
    │
    ▼
get_return_object() ──→ 返回给调用者的对象
    │
    ▼
initial_suspend() ──→ suspend_never：立即开始执行
                      suspend_always：挂起，等待手动 resume
    │
    ▼
协程函数体开始执行
    │
    ├── 遇到 co_await ──→ 挂起，控制权返回调用者
    │       │
    │       └── resume() ──→ 从挂起点继续
    │
    ├── 遇到 co_yield value ──→ 产出值，挂起
    │
    └── 遇到 co_return value / 函数末尾
            │
            ▼
        final_suspend() ──→ 决定最后是否挂起
            │
            ▼
        promise_type 析构，协程帧销毁
```

---

## 5. 第一个协程

```cpp
// 见 module1_basics/main.cpp
```

运行 module1 的三个示例：
1. `SimpleTask` — 最小协程，不挂起
2. `ManualTask` — 手动 resume 控制
3. `CountdownTask` — 用 co_await 演示暂停/恢复

---

## 6. 关键概念速查

| 概念 | 说明 |
|------|------|
| 协程帧 | 堆上分配的结构，保存局部变量和挂起状态 |
| `promise_type` | 协程的策略对象，控制生命周期行为 |
| `coroutine_handle<P>` | 指向协程帧的句柄，可 resume/destroy |
| `suspend_always` | 内置 Awaitable，总是挂起 |
| `suspend_never` | 内置 Awaitable，从不挂起 |
| ReturnObject | 协程返回给调用者的对象（Task/Generator） |

---

## 常见误区

1. **协程不是线程**：协程切换发生在用户态，不涉及内核调度
2. **co_return 不是 return**：普通函数不能用 co_return，协程不能用 return（除了 return object 语法）
3. **协程帧在堆上**：不要在析构后访问已销毁的协程帧
4. **`final_suspend` 必须 `noexcept`**：析构路径不能抛异常

---

**下一步** → [模块2：Promise 类型](02_promise.md)
