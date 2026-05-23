# 模块 3：co_yield 与生成器

---

## 设计模式：迭代器模式 (Iterator Pattern)

Generator 是迭代器模式的协程实现：
- **Iterator**：按需（惰性）访问序列中的每个元素
- **Generator**：用 `co_yield` 产出元素，只在被请求时才计算下一个值
- 对调用者透明：用标准 range-for 遍历，与操作容器完全一致

```
调用者 (range-for)          Generator<T>
    │                           │
    │──── begin() ─────────────►│ resume → 执行到第一个 co_yield
    │◄─── Iterator ─────────────│ 挂起，返回 Iterator
    │                           │
    │──── *it (取值) ───────────►│ 读取 promise.current_value
    │◄─── value ────────────────│
    │                           │
    │──── ++it ─────────────────►│ resume → 执行到下一个 co_yield
    │                           │ 挂起
    │                           │
    │──── it == end? ───────────►│ handle.done() ?
    │◄─── bool ─────────────────│
```

---

## co_yield 的工作原理

`co_yield expr` 是以下代码的语法糖：

```cpp
co_await promise.yield_value(expr);
```

执行流程：
1. 调用 `promise.yield_value(value)` —— 通常将值存入 promise
2. `yield_value` 返回一个 Awaitable（通常是 `suspend_always`）
3. 协程挂起，控制权返回给调用者
4. 调用者通过 `coroutine_handle::resume()` 恢复协程

---

## Generator<T> 完整实现

见 `module3_generator/generator.h`，核心设计：

```cpp
template<typename T>
class Generator {
public:
    struct promise_type {
        std::optional<T> current_value;

        // 惰性启动：构造后不立即执行
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        // 存储值并挂起
        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};
        }

        void return_void() { current_value = std::nullopt; }
        void unhandled_exception() { /* 存储异常 */ }

        Generator get_return_object() {
            return Generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
    };

    // 支持 range-for 的迭代器
    class Iterator {
        std::coroutine_handle<promise_type> handle_;
        void advance() {
            handle_.resume();
            if (handle_.done()) handle_ = nullptr;
        }
    public:
        // ... begin/end/++/*/== 实现
    };
};
```

---

## 示例一览

### 1. 无限序列

```cpp
Generator<int> fibonacci() {
    long long a = 0, b = 1;
    while (true) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}

// 使用
for (int v : take(fibonacci(), 10)) {
    std::cout << v << " ";
}
// 0 1 1 2 3 5 8 13 21 34
```

### 2. 惰性范围

```cpp
Generator<int> range(int start, int end, int step = 1) {
    for (int i = start; i < end; i += step) {
        co_yield i;
    }
}

for (int v : range(0, 10, 2)) {
    std::cout << v << " ";  // 0 2 4 6 8
}
```

### 3. 惰性转换（Pipeline）

```cpp
// 惰性 map：对生成器的每个元素应用变换
Generator<int> map(Generator<int> src, std::function<int(int)> fn) {
    for (int v : src) co_yield fn(v);
}

// 链式使用
auto result = map(range(1, 6), [](int x) { return x * x; });
for (int v : result) std::cout << v << " ";  // 1 4 9 16 25
```

---

## 惰性求值的优势

| 方面 | std::vector | Generator |
|------|-------------|-----------|
| 内存 | 全部存储 | 只存一个值 |
| 计算时机 | 全部预先计算 | 按需计算 |
| 无限序列 | 不支持 | 支持 |
| 早退出 | 浪费计算 | 零浪费 |

---

## 注意事项

1. **Generator 不可复制**：持有 `coroutine_handle`，只能移动
2. **不要在多线程中共享**：Generator 不是线程安全的
3. **引用悬空**：`co_yield ref` 中的引用在挂起期间必须有效
4. **异常处理**：`unhandled_exception` 应存储并在迭代时重新抛出

---

**下一步** → [模块4：co_await 与 Awaitable](04_awaitable.md)
