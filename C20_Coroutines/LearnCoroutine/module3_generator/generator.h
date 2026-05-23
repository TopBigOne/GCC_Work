#pragma once
#include <coroutine>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>

// ─────────────────────────────────────────────────────────────────────────────
// 设计模式：迭代器模式 (Iterator Pattern)
//
// Generator<T> 是迭代器模式的协程实现：
//   • 对调用者：standard range-for，与 std::vector 体验相同
//   • 内部：co_yield 产出一个值后挂起，下次 ++ 时才计算下一个值
//   • 优势：无限序列、惰性求值、零多余内存分配
//
// co_yield expr  等价于：co_await promise.yield_value(expr)
// ─────────────────────────────────────────────────────────────────────────────

namespace m3 {

// ═══════════════════════════════════════════════════════════════════════════
// Generator<T>
// ═══════════════════════════════════════════════════════════════════════════
template<typename T>
class Generator {
public:
    // ── Promise ─────────────────────────────────────────────────────────────
    struct promise_type {
        std::optional<T>   current_value;
        std::exception_ptr exception;

        Generator get_return_object() noexcept {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend()   noexcept { return {}; }

        // co_yield expr → 存值并挂起
        std::suspend_always yield_value(T value) noexcept {
            current_value = std::move(value);
            return {};
        }

        void return_void()         noexcept { current_value = std::nullopt; }
        void unhandled_exception() noexcept { exception = std::current_exception(); }
    };

    // ── Iterator（实现迭代器协议）───────────────────────────────────────────
    class Iterator {
        std::coroutine_handle<promise_type> handle_;

        void check_exception() {
            if (handle_ && handle_.promise().exception)
                std::rethrow_exception(handle_.promise().exception);
        }

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T*;
        using reference         = const T&;

        Iterator() noexcept : handle_(nullptr) {}
        explicit Iterator(std::coroutine_handle<promise_type> h) noexcept : handle_(h) {}

        // ++ 推进到下一个值
        Iterator& operator++() {
            handle_.resume();
            if (handle_.done()) {
                check_exception();
                handle_ = nullptr;  // 变成 end
            }
            return *this;
        }

        reference operator*()  const noexcept { return *handle_.promise().current_value; }
        pointer   operator->() const noexcept { return &*handle_.promise().current_value; }

        bool operator==(const Iterator& rhs) const noexcept { return handle_ == rhs.handle_; }
        bool operator!=(const Iterator& rhs) const noexcept { return !(*this == rhs); }
    };

    // ── Generator 本体 ───────────────────────────────────────────────────────
    explicit Generator(std::coroutine_handle<promise_type> h) noexcept : handle_(h) {}
    Generator(const Generator&)            = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }
    Generator& operator=(Generator&&) = delete;
    ~Generator() { if (handle_) handle_.destroy(); }

    Iterator begin() {
        handle_.resume();  // 推进到第一个 co_yield
        if (handle_.done()) return end();
        return Iterator{handle_};
    }
    Iterator end() noexcept { return Iterator{}; }

private:
    std::coroutine_handle<promise_type> handle_;
};

// ═══════════════════════════════════════════════════════════════════════════
// 具体生成器函数
// ═══════════════════════════════════════════════════════════════════════════

// 无限斐波那契数列
inline Generator<long long> fibonacci() {
    long long a = 0, b = 1;
    while (true) {
        co_yield a;
        long long next = a + b;
        a = b;
        b = next;
    }
}

// 惰性整数范围 [start, stop) step
inline Generator<int> range(int start, int stop, int step = 1) {
    for (int i = start; i < stop; i += step)
        co_yield i;
}

// 惰性 map：对每个元素应用变换
inline Generator<int> map_gen(Generator<int> src, std::function<int(int)> fn) {
    for (int v : src) co_yield fn(v);
}

// 惰性 filter：只产出满足谓词的元素
inline Generator<int> filter_gen(Generator<int> src, std::function<bool(int)> pred) {
    for (int v : src)
        if (pred(v)) co_yield v;
}

// 取前 N 个元素
template<typename T>
Generator<T> take(Generator<T> src, int n) {
    int count = 0;
    for (T v : src) {
        if (count++ >= n) break;
        co_yield v;
    }
}

// ───────────────────────────────────────────────────────────────────────────
inline void run_module3() {
    std::cout << "\n╔══ M3 co_yield 与生成器（迭代器模式）════════╗\n";

    std::cout << "\n── Demo 1: fibonacci — 无限惰性数列（取前 12 项）──\n  ";
    auto fib = fibonacci();
    int cnt = 0;
    for (long long v : fib) {
        std::cout << v << " ";
        if (++cnt >= 12) break;
    }
    std::cout << "\n";

    std::cout << "\n── Demo 2: range(0, 10, 2) — 惰性步进范围 ──\n  ";
    for (int v : range(0, 10, 2)) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "\n── Demo 3: map_gen — 惰性变换（x²）──\n  ";
    auto mapped = map_gen(range(1, 6), [](int x) { return x * x; });
    for (int v : mapped) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "\n── Demo 4: filter_gen — 惰性过滤（偶数）──\n  ";
    auto filtered = filter_gen(range(1, 11), [](int x) { return x % 2 == 0; });
    for (int v : filtered) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "\n── Demo 5: Pipeline — range → filter → map ──\n  ";
    // 1..20 中的奇数，乘以 3
    auto pipeline = map_gen(
        filter_gen(range(1, 20), [](int x) { return x % 2 != 0; }),
        [](int x) { return x * 3; }
    );
    for (int v : pipeline) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "╚═════════════════════════════════════════════╝\n";
}

} // namespace m3
