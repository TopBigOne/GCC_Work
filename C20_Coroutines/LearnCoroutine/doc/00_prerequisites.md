# 前置准备：C++11/17 必备基础

学习 C++20 协程前，需要牢固掌握以下知识点。每个小节提供最小可运行示例。

---

## 1. Lambda 与闭包

协程的 Awaitable / Promise 大量用到 lambda 作为回调。

```cpp
#include <functional>
#include <iostream>

int main() {
    int x = 10;

    // 值捕获
    auto f1 = [x]() { return x * 2; };

    // 引用捕获（注意悬空引用风险）
    auto f2 = [&x]() { x++; };

    // 泛型 lambda (C++14)
    auto add = [](auto a, auto b) { return a + b; };

    f2();
    std::cout << x << "\n";           // 11
    std::cout << f1() << "\n";        // 20（捕获的是修改前的 x=10）
    std::cout << add(1, 2.5) << "\n"; // 3.5
}
```

**关键点**：
- 值捕获是拍快照，引用捕获共享变量
- 协程挂起后 lambda 中的引用捕获可能悬空 —— 后续模块会专门讲

---

## 2. std::function

```cpp
#include <functional>
#include <iostream>

void execute(std::function<void(int)> callback) {
    callback(42);
}

int main() {
    // 可以存任何可调用对象
    std::function<void(int)> f = [](int v) {
        std::cout << "value: " << v << "\n";
    };

    execute(f);          // value: 42
    execute([](int v) {
        std::cout << "inline: " << v << "\n";
    });
}
```

**协程中的用途**：存储恢复回调（continuation）。

---

## 3. 移动语义 (Move Semantics)

协程句柄（`coroutine_handle`）是不可复制、只能移动的资源。

```cpp
#include <iostream>
#include <string>
#include <utility>

struct Resource {
    std::string name;

    Resource(std::string n) : name(std::move(n)) {
        std::cout << "构造: " << name << "\n";
    }

    // 移动构造：转移所有权，source 变为空
    Resource(Resource&& other) noexcept : name(std::move(other.name)) {
        std::cout << "移动构造\n";
    }

    // 禁止拷贝
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;

    ~Resource() {
        if (!name.empty()) std::cout << "析构: " << name << "\n";
    }
};

int main() {
    Resource r1("handle");
    Resource r2(std::move(r1));  // r1 被掏空
    // r1.name 现在为空，不会在析构时打印
}
```

---

## 4. 模板基础

Promise type 和 Generator 都是类模板。

```cpp
#include <iostream>

// 函数模板
template<typename T>
T max_value(T a, T b) { return a > b ? a : b; }

// 类模板
template<typename T>
class Box {
public:
    explicit Box(T v) : value_(std::move(v)) {}
    T get() const { return value_; }
private:
    T value_;
};

// 模板特化
template<>
class Box<bool> {
public:
    explicit Box(bool v) : value_(v) {}
    bool get() const { return value_; }
    void toggle() { value_ = !value_; }
private:
    bool value_;
};

int main() {
    std::cout << max_value(3, 5) << "\n";        // 5
    Box<int> b(42);
    std::cout << b.get() << "\n";               // 42
    Box<bool> flag(true);
    flag.toggle();
    std::cout << flag.get() << "\n";            // 0
}
```

**重点**：
- `template<typename T>` 声明
- 类型推导
- 特化（后续 Promise type 会用到）

---

## 5. RAII 与智能指针

协程帧（coroutine frame）由运行时管理，类似 `unique_ptr` 的所有权语义。

```cpp
#include <memory>
#include <iostream>

struct Connection {
    Connection() { std::cout << "建立连接\n"; }
    ~Connection() { std::cout << "关闭连接\n"; }
    void send(const std::string& msg) {
        std::cout << "发送: " << msg << "\n";
    }
};

int main() {
    // unique_ptr: 独占所有权，离开作用域自动销毁
    auto conn = std::make_unique<Connection>();
    conn->send("hello");

    // 转移所有权（类似 coroutine_handle 的移动）
    auto conn2 = std::move(conn);
    // conn 现在为 nullptr
}
// conn2 在这里析构 → 打印"关闭连接"
```

---

## 6. std::optional (C++17)

Generator 内部用 `optional` 存储当前产出的值。

```cpp
#include <optional>
#include <iostream>
#include <string>

std::optional<std::string> find_user(int id) {
    if (id == 1) return "Alice";
    return std::nullopt;  // 无值
}

int main() {
    auto result = find_user(1);
    if (result) {
        std::cout << *result << "\n";         // Alice
        std::cout << result.value() << "\n";  // Alice
    }

    auto missing = find_user(99);
    std::cout << missing.value_or("Unknown") << "\n"; // Unknown
}
```

---

## 7. if constexpr (C++17)

用于在模板中根据类型做编译期分支。

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void print_type_info(T val) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "整数: " << val << "\n";
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮点: " << val << "\n";
    } else {
        std::cout << "其他类型\n";
    }
}

int main() {
    print_type_info(42);       // 整数: 42
    print_type_info(3.14);     // 浮点: 3.14
    print_type_info("hello");  // 其他类型
}
```

---

## 检查清单

学完本节，你应该能：

- [ ] 写出带值捕获和引用捕获的 lambda
- [ ] 用 `std::function` 存储回调
- [ ] 理解移动构造的作用，知道何时用 `std::move`
- [ ] 写一个简单的类模板
- [ ] 用 `std::optional` 表示可能为空的返回值
- [ ] 理解 RAII：资源生命周期与对象生命周期绑定

---

**下一步** → [模块1：协程基础概念](01_basics.md)
