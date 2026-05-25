# 学习 C++ std::exception_ptr

## 1. 概述

`std::exception_ptr` 是 C++11 引入的智能指针类型，用于**跨线程、跨作用域传递异常**。它可以捕获当前异常并保存，稍后在任意位置重新抛出。

核心能力：
- 捕获异常对象并延迟重新抛出
- 跨线程传递异常（`std::thread` / `std::async` 的基础）
- 空状态表示"无异常"（类似 `nullptr`）

---

## 2. 核心 API

| API | 说明 |
|-----|------|
| `std::exception_ptr` | 异常指针类型（可为空） |
| `std::current_exception()` | 在 catch 块中捕获当前异常，返回 `exception_ptr` |
| `std::rethrow_exception(ep)` | 重新抛出 `exception_ptr` 所持有的异常 |
| `std::make_exception_ptr(e)` | 直接从异常对象创建 `exception_ptr`，无需 throw/catch |

---

## 3. 基础用法

### 3.1 捕获并延迟重新抛出

```cpp
#include <iostream>
#include <exception>
#include <stdexcept>

int main() {
    std::exception_ptr ep;

    // 捕获异常，保存起来
    try {
        throw std::runtime_error("something went wrong");
    } catch (...) {
        ep = std::current_exception(); // 保存当前异常
    }

    std::cout << "异常已被保存，继续执行..." << std::endl;

    // 稍后重新抛出
    if (ep) {
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            std::cerr << "重新捕获: " << e.what() << std::endl;
        }
    }

    return 0;
}
```

输出：
```
异常已被保存，继续执行...
重新捕获: something went wrong
```

### 3.2 空状态判断

```cpp
std::exception_ptr ep; // 默认为空，表示无异常

if (!ep) {
    std::cout << "无异常" << std::endl;
}

// 赋值为 nullptr 可以清除
ep = nullptr;
```

### 3.3 make_exception_ptr（无需 throw）

```cpp
// 不需要进入 catch 块，直接创建
std::exception_ptr ep = std::make_exception_ptr(std::runtime_error("direct error"));

try {
    std::rethrow_exception(ep);
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl; // direct error
}
```

---

## 4. 跨线程传递异常（核心应用场景）

`std::exception_ptr` 最重要的用途是**将子线程的异常传递给主线程**。

```cpp
#include <iostream>
#include <thread>
#include <exception>
#include <stdexcept>

std::exception_ptr g_ep = nullptr;

void worker() {
    try {
        throw std::runtime_error("子线程发生异常");
    } catch (...) {
        g_ep = std::current_exception(); // 保存异常
    }
}

int main() {
    std::thread t(worker);
    t.join();

    // 主线程处理子线程的异常
    if (g_ep) {
        try {
            std::rethrow_exception(g_ep);
        } catch (const std::exception& e) {
            std::cerr << "主线程捕获: " << e.what() << std::endl;
        }
    }

    return 0;
}
```

输出：
```
主线程捕获: 子线程发生异常
```

> **注意**：C++ 标准规定，异常不能跨线程自动传播，必须借助 `exception_ptr` 手动传递。

---

## 5. 与 std::future / std::async 的关系

`std::async` 内部正是使用 `exception_ptr` 来捕获异步任务中的异常，并在调用 `future::get()` 时重新抛出。

```cpp
#include <iostream>
#include <future>
#include <stdexcept>

int main() {
    auto fut = std::async(std::launch::async, []() -> int {
        throw std::runtime_error("async 任务异常");
        return 42;
    });

    try {
        int result = fut.get(); // 此处重新抛出子线程异常
    } catch (const std::exception& e) {
        std::cerr << "捕获 async 异常: " << e.what() << std::endl;
    }

    return 0;
}
```

输出：
```
捕获 async 异常: async 任务异常
```

---

## 6. 存储多个线程的异常

```cpp
#include <iostream>
#include <thread>
#include <exception>
#include <vector>
#include <stdexcept>

int main() {
    const int N = 4;
    std::vector<std::exception_ptr> exceptions(N);
    std::vector<std::thread> threads;

    for (int i = 0; i < N; ++i) {
        threads.emplace_back([i, &exceptions]() {
            try {
                if (i % 2 == 0) {
                    throw std::runtime_error("线程 " + std::to_string(i) + " 失败");
                }
            } catch (...) {
                exceptions[i] = std::current_exception();
            }
        });
    }

    for (auto& t : threads) t.join();

    // 统一处理所有异常
    for (int i = 0; i < N; ++i) {
        if (exceptions[i]) {
            try {
                std::rethrow_exception(exceptions[i]);
            } catch (const std::exception& e) {
                std::cerr << "线程 " << i << " 异常: " << e.what() << std::endl;
            }
        }
    }

    return 0;
}
```

---

## 7. 注意事项

### 7.1 rethrow_exception 必须在 try 块中调用

```cpp
// 错误：没有捕获，会直接终止程序
std::rethrow_exception(ep); // 未被 try/catch 包裹 -> std::terminate()

// 正确
try {
    std::rethrow_exception(ep);
} catch (...) { ... }
```

### 7.2 current_exception() 只能在 catch 块中调用

```cpp
// 在 catch 块外调用返回空指针
std::exception_ptr ep = std::current_exception(); // 返回 nullptr（无意义）

try {
    throw std::runtime_error("err");
} catch (...) {
    ep = std::current_exception(); // 正确
}
```

### 7.3 exception_ptr 延长异常对象生命周期

`exception_ptr` 持有异常对象的共享所有权，只要 `exception_ptr` 存活，异常对象就不会被销毁。

### 7.4 线程安全

`exception_ptr` 本身的复制和赋值是线程安全的，但如果多个线程写同一个 `exception_ptr` 变量，需要加锁保护。

---

## 8. 设计模式：异常聚合器

将多个任务的异常统一收集，最终一次性处理：

```cpp
#include <iostream>
#include <exception>
#include <vector>
#include <stdexcept>
#include <functional>

class ExceptionAggregator {
    std::vector<std::exception_ptr> errors_;
public:
    void run(std::function<void()> task) {
        try {
            task();
        } catch (...) {
            errors_.push_back(std::current_exception());
        }
    }

    void rethrow_all() {
        for (auto& ep : errors_) {
            try {
                std::rethrow_exception(ep);
            } catch (const std::exception& e) {
                std::cerr << "[聚合异常] " << e.what() << std::endl;
            }
        }
    }

    bool has_errors() const { return !errors_.empty(); }
};

int main() {
    ExceptionAggregator agg;

    agg.run([] { throw std::runtime_error("任务1失败"); });
    agg.run([] { /* 成功 */ });
    agg.run([] { throw std::logic_error("任务3逻辑错误"); });

    if (agg.has_errors()) {
        agg.rethrow_all();
    }

    return 0;
}
```

---

## 9. 总结

| 特性 | 说明 |
|------|------|
| 类型 | 类似 `shared_ptr` 的智能指针，引用计数管理异常对象 |
| 空状态 | 默认构造为空，等价于 `nullptr` |
| 线程传递 | 跨线程传递异常的标准方式 |
| 延迟抛出 | 捕获后可在任意时机、任意位置重新抛出 |
| 底层支撑 | `std::promise`、`std::future`、`std::async` 的异常传播机制 |

```
捕获异常          保存               重新抛出
catch(...) --> current_exception() --> exception_ptr --> rethrow_exception()
```
