# C++20 协程学习计划

> 适合人群：有一定 C++ 基础，希望掌握 C++20 协程并能在项目中实际使用。

---

## 前置准备

在学协程之前，需要补齐以下 C++11/17 基础：

| 知识点 | 说明 |
|--------|------|
| `std::function` / lambda | 协程常与回调结合 |
| 模板基础 | Promise / Awaitable 都依赖模板 |
| 移动语义 (`std::move`) | 协程句柄涉及资源转移 |
| `std::optional` | 协程返回值常用 |
| RAII / 智能指针 | 协程生命周期管理 |

---

## 学习路线（6 个模块）

### 模块 1：协程基础概念
**目标**：理解协程是什么，与线程/函数的区别

- 协程 vs 普通函数 vs 线程
- 协程的三个关键字：`co_await` / `co_yield` / `co_return`
- 协程的挂起（suspend）与恢复（resume）
- 协程在 C++20 中的整体架构图

**Demo**：写第一个能编译运行的协程（用第三方库如 cppcoro 或手写最简 Promise）

---

### 模块 2：Promise 类型
**目标**：理解协程框架的核心 — Promise

- `promise_type` 的作用和生命周期
- 必须实现的接口：
  - `get_return_object()`
  - `initial_suspend()` / `final_suspend()`
  - `return_void()` / `return_value()`
  - `unhandled_exception()`
- `std::suspend_always` vs `std::suspend_never`

**Demo**：手写最简单的 `Task<void>` 类型

---

### 模块 3：co_yield 与生成器
**目标**：实现惰性序列生成器

- `co_yield` 的工作原理（等价于 `co_await promise.yield_value(expr)`）
- `yield_value()` 在 Promise 中的实现
- 实现 `Generator<T>`：支持 range-for 的惰性生成器

**Demo**：
- 无限斐波那契数列生成器
- 文件按行懒读取生成器

---

### 模块 4：co_await 与 Awaitable
**目标**：掌握协程挂起/恢复的控制机制

- Awaitable 协议：`await_ready()` / `await_suspend()` / `await_resume()`
- `await_suspend` 的三种返回类型（`void` / `bool` / `coroutine_handle<>`）
- `coroutine_handle<>` 的使用
- 自定义 Awaitable 类型

**Demo**：
- 实现 `SwitchToThread`（切换执行线程）
- 实现简单的定时器 Awaitable

---

### 模块 5：异步任务 Task<T>
**目标**：实现可 co_await 的异步任务

- `Task<T>` 完整实现（支持链式 co_await）
- 协程句柄的传递与恢复（continuation）
- 异常在协程中的传播
- 协程的取消（cancellation）

**Demo**：
- 模拟异步文件读取
- 多个 Task 串联执行（async/await 风格）

---

### 模块 6：实战应用
**目标**：在真实场景中使用协程

- 协程 + 线程池：实现简单的协程调度器
- 协程 + 网络IO（结合 epoll/kqueue 或 asio）
- 与标准库 `std::jthread` 结合
- 常见陷阱：悬空引用、生命周期问题、栈帧大小

**Demo**：
- 实现一个简单的协程任务调度器
- （可选）集成 Boost.Asio 的协程支持

---

## 每个模块的学习结构

每个模块按以下步骤推进：

1. 阅读概念 → 画图理解执行流
2. 看最小 Demo → 手动单步调试
3. 自己实现一遍
4. 写单元测试验证行为
5. 记录关键点到本文档

---

## 推荐参考资料

| 资源 | 说明 |
|------|------|
| cppreference - Coroutines | 官方规范，随时查阅 |
| Lewis Baker 的博客系列 | 目前最好的 C++20 协程深度讲解 |
| cppcoro 库源码 | 工业级协程原语参考实现 |
| CppCon 2019: "A Unifying Abstraction for Async in C++" | Lewis Baker 演讲视频 |

---

## 项目结构规划

```
LearnCoroutine/
├── CMakeLists.txt
├── readmd.md
├── main.cpp
├── module1_basics/        # 模块1：基础概念
├── module2_promise/       # 模块2：Promise 类型
├── module3_generator/     # 模块3：co_yield 生成器
├── module4_awaitable/     # 模块4：co_await 机制
├── module5_task/          # 模块5：异步 Task
└── module6_realworld/     # 模块6：实战应用
```

---

## 学习进度

- [ ] 前置准备：补齐 C++11/17 基础
- [ ] 模块 1：协程基础概念
- [ ] 模块 2：Promise 类型
- [ ] 模块 3：co_yield 与生成器
- [ ] 模块 4：co_await 与 Awaitable
- [ ] 模块 5：异步任务 Task<T>
- [ ] 模块 6：实战应用
