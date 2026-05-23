#pragma once
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

// ─────────────────────────────────────────────────────────────────────────────
// C++11/17 前置基础 — 协程学习必备
//
// 知识点列表：
//   P1  Lambda 与闭包           → 协程 Awaitable 大量用回调
//   P2  std::function           → 存储 continuation（恢复回调）
//   P3  移动语义                → coroutine_handle 只能移动，不能复制
//   P4  模板基础                → promise_type / Generator<T> 都是模板
//   P5  RAII 与智能指针         → 协程帧生命周期管理
//   P6  std::optional (C++17)   → Generator 内部存当前值
//   P7  if constexpr (C++17)    → 模板中编译期分支
//   P8  std::variant (C++17)    → Task<T> 存储"值或异常"两种状态
// ─────────────────────────────────────────────────────────────────────────────

namespace m0 {

// ═══════════════════════════════════════════════════════════════════════════
// P1: Lambda 与闭包
// ═══════════════════════════════════════════════════════════════════════════
inline void demo_lambda() {
    std::cout << "\n── P1: Lambda 与闭包 ──\n";

    int x = 10;

    // 值捕获：拍快照，不受后续修改影响
    auto by_value = [x]() { return x * 2; };

    // 引用捕获：共享变量，后续修改可见
    auto by_ref = [&x]() { x += 5; };

    // mutable：值捕获后允许在 lambda 内部修改副本
    auto mutable_copy = [x]() mutable { x += 100; return x; };

    // 泛型 lambda (C++14)：auto 参数，适用于任意类型
    auto add = [](auto a, auto b) { return a + b; };

    // 捕获列表 + 立即执行（IIFE）
    int result = [&x]() { return x * 3; }();

    by_ref();
    std::cout << "  x after by_ref()     = " << x << "\n";         // 15
    std::cout << "  by_value()           = " << by_value() << "\n"; // 20（x=10的快照）
    std::cout << "  mutable_copy()       = " << mutable_copy() << "\n"; // 110（快照x=10，副本+100）
    std::cout << "  x after mutable_copy = " << x << "\n";         // 15（原 x 不变）
    std::cout << "  add(1, 2.5)          = " << add(1, 2.5) << "\n"; // 3.5
    std::cout << "  IIFE result          = " << result << "\n";     // 30（IIFE执行时x=10）

    // ↓ 协程相关：把 lambda 作为回调存起来
    std::function<void()> callback = [x]() {
        std::cout << "  [callback] 异步回调执行，x=" << x << "\n";
    };
    callback();
}

// ═══════════════════════════════════════════════════════════════════════════
// P2: std::function — 类型擦除的可调用对象包装器
// ═══════════════════════════════════════════════════════════════════════════

// 模拟异步操作：完成后调用 callback
inline void async_op(int value, std::function<void(int)> on_done) {
    // 真实场景：这里会启动 IO，回调在 IO 完成后被调用
    // 演示中直接调用
    on_done(value * 2);
}

inline void demo_function() {
    std::cout << "\n── P2: std::function ──\n";

    // 包装普通函数
    auto square = [](int v) { return v * v; };
    std::function<int(int)> fn = square;
    std::cout << "  fn(7)  = " << fn(7) << "\n";  // 49

    // 重新绑定为另一个 lambda
    fn = [](int v) { return v + 100; };
    std::cout << "  fn(7)  = " << fn(7) << "\n";  // 107（换了实现）

    // 用作回调参数（协程中 continuation 的雏形）
    async_op(21, [](int result) {
        std::cout << "  [async_op] 结果 = " << result << "\n";  // 42
    });

    // 存储为成员变量（调度器内部的常见用法）
    std::vector<std::function<void()>> tasks;
    for (int i = 0; i < 3; ++i) {
        tasks.push_back([i]() {
            std::cout << "  [task " << i << "] 执行\n";
        });
    }
    for (auto& t : tasks) t();
}

// ═══════════════════════════════════════════════════════════════════════════
// P3: 移动语义
// coroutine_handle 是只移动资源：类似 unique_ptr，转移后原对象失效
// ═══════════════════════════════════════════════════════════════════════════
struct Handle {
    std::string name_;
    bool valid_ = false;

    explicit Handle(std::string n) : name_(std::move(n)), valid_(true) {
        std::cout << "  [Handle] 构造: " << name_ << "\n";
    }

    // 移动构造：转移所有权，source 标记为无效
    Handle(Handle&& o) noexcept
        : name_(std::move(o.name_)), valid_(o.valid_)
    {
        o.valid_ = false;
        std::cout << "  [Handle] 移动构造 → " << name_ << "\n";
    }

    // 禁止复制（coroutine_handle 同款语义）
    Handle(const Handle&)            = delete;
    Handle& operator=(const Handle&) = delete;

    // 移动赋值
    Handle& operator=(Handle&& o) noexcept {
        if (this != &o) {
            name_  = std::move(o.name_);
            valid_ = o.valid_;
            o.valid_ = false;
        }
        return *this;
    }

    ~Handle() {
        if (valid_) std::cout << "  [Handle] 析构: " << name_ << "\n";
        else        std::cout << "  [Handle] 析构（已被移走）\n";
    }

    void use() const {
        if (!valid_) { std::cout << "  [Handle] 错误：使用已移走的 handle!\n"; return; }
        std::cout << "  [Handle] 使用: " << name_ << "\n";
    }
};

inline void demo_move() {
    std::cout << "\n── P3: 移动语义 ──\n";

    Handle h1("coroutine-frame-1");
    h1.use();

    Handle h2(std::move(h1));  // 移动，h1 失效
    std::cout << "  h1 valid after move = " << std::boolalpha << h1.valid_ << "\n";
    h2.use();

    // std::move 不移动，只是"标记可以移动"
    // 实际移动发生在移动构造/赋值里
    Handle h3("coroutine-frame-3");
    h3 = std::move(h2);
    h3.use();

    // 函数返回时自动触发 NRVO 或移动（不需要手写 std::move）
}  // h1(已空)、h2(已被移走)、h3 在这里析构

// ═══════════════════════════════════════════════════════════════════════════
// P4: 模板基础
// ═══════════════════════════════════════════════════════════════════════════

// 函数模板：类型自动推导
template<typename T>
T clamp(T val, T lo, T hi) {
    return val < lo ? lo : (val > hi ? hi : val);
}

// 类模板：协程 Generator<T> / Task<T> 的基础
template<typename T>
class Box {
public:
    explicit Box(T v) : value_(std::move(v)) {}

    T get() const { return value_; }
    void set(T v) { value_ = std::move(v); }

    // 成员函数模板：转换为另一种类型的 Box
    template<typename U>
    Box<U> transform(std::function<U(T)> fn) const {
        return Box<U>{fn(value_)};
    }

private:
    T value_;
};

// 显式特化：bool 版 Box 多一个 toggle（Task<void> 特化的类比）
template<>
class Box<bool> {
public:
    explicit Box(bool v) : value_(v) {}
    bool get() const { return value_; }
    void toggle() { value_ = !value_; }
private:
    bool value_;
};

inline void demo_template() {
    std::cout << "\n── P4: 模板基础 ──\n";

    // 函数模板：类型推导
    std::cout << "  clamp(15, 0, 10) = " << clamp(15, 0, 10) << "\n";   // 10
    std::cout << "  clamp(3.5, 1.0, 5.0) = " << clamp(3.5, 1.0, 5.0) << "\n"; // 3.5

    // 类模板
    Box<int> b1{42};
    auto b2 = b1.transform<std::string>([](int v) {
        return "value=" + std::to_string(v);
    });
    std::cout << "  Box<int>::get()    = " << b1.get() << "\n";
    std::cout << "  Box<string>::get() = " << b2.get() << "\n";

    // 特化版本
    Box<bool> flag{true};
    flag.toggle();
    std::cout << "  Box<bool> after toggle = " << flag.get() << "\n";  // 0
}

// ═══════════════════════════════════════════════════════════════════════════
// P5: RAII 与智能指针
// ═══════════════════════════════════════════════════════════════════════════
struct NetworkConn {
    std::string endpoint;
    NetworkConn(std::string ep) : endpoint(std::move(ep)) {
        std::cout << "  [Conn] 建立连接: " << endpoint << "\n";
    }
    ~NetworkConn() {
        std::cout << "  [Conn] 关闭连接: " << endpoint << "\n";
    }
    void send(std::string_view msg) const {
        std::cout << "  [Conn] 发送 → " << msg << "\n";
    }
};

inline void demo_raii() {
    std::cout << "\n── P5: RAII 与智能指针 ──\n";

    // unique_ptr：独占所有权，析构时自动 delete
    // → coroutine_handle 的生命周期管理方式完全一致
    {
        auto conn = std::make_unique<NetworkConn>("localhost:8080");
        conn->send("hello");

        // 转移所有权（coroutine_handle 移动语义）
        auto conn2 = std::move(conn);
        // conn 现在是 nullptr，conn2 拥有连接
        std::cout << "  conn  == nullptr: " << (conn == nullptr ? "yes" : "no") << "\n";
        conn2->send("world");
    }  // conn2 析构 → 自动关闭连接

    // shared_ptr：引用计数，最后一个 owner 析构时 delete
    std::cout << "  ---\n";
    {
        auto sp1 = std::make_shared<NetworkConn>("remote:9090");
        {
            auto sp2 = sp1;  // 引用计数 +1
            std::cout << "  use_count = " << sp1.use_count() << "\n";  // 2
            sp2->send("from sp2");
        }  // sp2 析构，引用计数 -1 → 不销毁
        std::cout << "  use_count = " << sp1.use_count() << "\n";  // 1
    }  // sp1 析构，引用计数 0 → 销毁连接
}

// ═══════════════════════════════════════════════════════════════════════════
// P6: std::optional (C++17)
// Generator<T> 内部用 optional<T> 存当前产出的值
// ═══════════════════════════════════════════════════════════════════════════

// 模拟"可能失败的查找"
inline std::optional<std::string> find_user(int id) {
    if (id == 1) return "Alice";
    if (id == 2) return "Bob";
    return std::nullopt;
}

// 链式处理（类似 Rust Option::and_then）
inline std::optional<int> name_length(int id) {
    auto user = find_user(id);
    if (!user) return std::nullopt;
    return static_cast<int>(user->size());
}

inline void demo_optional() {
    std::cout << "\n── P6: std::optional (C++17) ──\n";

    // 基本用法
    auto u1 = find_user(1);
    auto u2 = find_user(99);

    std::cout << "  find_user(1) has_value = " << u1.has_value() << "\n";
    std::cout << "  find_user(1) value     = " << *u1 << "\n";          // Alice
    std::cout << "  find_user(99) or def   = " << u2.value_or("Unknown") << "\n"; // Unknown

    // 安全访问
    if (auto u = find_user(2)) {
        std::cout << "  find_user(2)           = " << *u << "\n";  // Bob
    }

    // 链式（Generator 内部 current_value 的典型用法）
    for (int id : {1, 2, 99}) {
        auto len = name_length(id);
        std::cout << "  name_length(" << id << ") = "
                  << (len ? std::to_string(*len) : "nullopt") << "\n";
    }
    // → 5, 3, nullopt

    // optional 在协程中：
    // promise_type::current_value 在 co_yield 时赋值，return_void 时置 nullopt
    std::optional<int> current_value;
    current_value = 42;
    std::cout << "  current_value = " << *current_value << "\n";
    current_value = std::nullopt;  // 相当于协程 return_void
    std::cout << "  current_value has_value = " << current_value.has_value() << "\n";
}

// ═══════════════════════════════════════════════════════════════════════════
// P7: if constexpr (C++17)
// 用于模板中的编译期分支，避免运行时开销
// ═══════════════════════════════════════════════════════════════════════════
template<typename T>
std::string describe(T val) {
    if constexpr (std::is_void_v<T>) {
        // Task<void> 的 promise_type 特化：return_void vs return_value
        return "void（用于 Task<void> 特化）";
    } else if constexpr (std::is_integral_v<T>) {
        return "整数: " + std::to_string(val);
    } else if constexpr (std::is_floating_point_v<T>) {
        return "浮点: " + std::to_string(val);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "字符串: " + val;
    } else {
        return "其他类型";
    }
}

// 编译期选择 return_void / return_value（Task<T> promise_type 的核心技巧）
template<typename T>
struct PromiseLike {
    void set_result(T val) {
        if constexpr (std::is_void_v<T>) {
            std::cout << "  [Promise] return_void() — 无返回值\n";
        } else {
            std::cout << "  [Promise] return_value(" << val << ")\n";
        }
    }
};

inline void demo_if_constexpr() {
    std::cout << "\n── P7: if constexpr (C++17) ──\n";

    std::cout << "  describe(42)       = " << describe(42) << "\n";
    std::cout << "  describe(3.14)     = " << describe(3.14) << "\n";
    std::cout << "  describe(\"hello\"s) = " << describe(std::string("hello")) << "\n";
    std::cout << "  describe(nullptr)  = " << describe(nullptr) << "\n";

    // 模拟 Task<T> 的 promise 在 void/非void 上的编译期分支
    PromiseLike<int>  p1; p1.set_result(99);
    // PromiseLike<void> p2; p2.set_result(???);
    // 因为 void 无法传参，实际要结合特化，这里仅展示 if constexpr 的用途
}

// ═══════════════════════════════════════════════════════════════════════════
// P8: std::variant (C++17) — 类型安全的联合体
// Task<T> 的 promise_type 用 variant<monostate, T, exception_ptr> 存结果
// ═══════════════════════════════════════════════════════════════════════════
inline void demo_variant() {
    std::cout << "\n── P8: std::variant (C++17) ──\n";

    // 三态：未完成 / 有值 / 有异常（Task<T> 的 result_ 字段就是这个）
    using TaskResult = std::variant<std::monostate, int, std::exception_ptr>;

    // 状态 1：未完成
    TaskResult r1{};
    std::cout << "  初始状态 (monostate): index=" << r1.index() << "\n";  // 0

    // 状态 2：有返回值
    TaskResult r2{42};
    std::cout << "  有值状态: " << std::get<int>(r2) << "\n";  // 42

    // 状态 3：有异常
    try {
        throw std::runtime_error("模拟异步错误");
    } catch (...) {
        TaskResult r3{std::current_exception()};
        // 读取时用 std::get_if 安全访问
        if (auto* ep = std::get_if<std::exception_ptr>(&r3)) {
            try {
                std::rethrow_exception(*ep);
            } catch (const std::exception& e) {
                std::cout << "  异常状态: " << e.what() << "\n";
            }
        }
    }

    // std::visit：访问者模式，匹配当前持有类型
    auto print_result = [](const TaskResult& r) {
        std::visit([](auto&& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                std::cout << "  [visit] 未完成\n";
            } else if constexpr (std::is_same_v<T, int>) {
                std::cout << "  [visit] 返回值 = " << v << "\n";
            } else {
                std::cout << "  [visit] 有异常\n";
            }
        }, r);
    };

    print_result(r1);   // 未完成
    print_result(r2);   // 返回值 = 42
}

// ───────────────────────────────────────────────────────────────────────────
inline void run_module0() {
    std::cout << "\n╔══ M0 C++11/17 前置基础 ══════════════════════╗\n";

    demo_lambda();
    demo_function();
    demo_move();
    demo_template();
    demo_raii();
    demo_optional();
    demo_if_constexpr();
    demo_variant();

    std::cout << "╚═════════════════════════════════════════════╝\n";
}

} // namespace m0
