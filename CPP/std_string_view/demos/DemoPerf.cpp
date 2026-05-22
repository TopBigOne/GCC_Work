#include "DemoPerf.h"
#include <string_view>
#include <string>
#include <chrono>
#include <cstdio>

// ════════════════════════════════════════════════════════════════
// 具体策略实现（Strategy 模式）
// 三种策略的核心逻辑完全相同（统计字符数），区别仅在参数类型。
// ════════════════════════════════════════════════════════════════

// ── 策略 A：std::string_view（现代推荐，零拷贝）─────────────────
class StringViewBench : public IStringBench {
public:
    std::string strategyName() const override {
        return "std::string_view";
    }

    static size_t countChars(std::string_view sv) {
        size_t n = 0;
        for (char c : sv) n += (c != ' ');
        return n;
    }

    double benchmark(const std::string& data, int repeat) const override {
        auto t0 = std::chrono::high_resolution_clock::now();
        volatile size_t result = 0;
        for (int i = 0; i < repeat; ++i)
            result = countChars(data);   // 隐式转换为 string_view，无堆分配
        auto t1 = std::chrono::high_resolution_clock::now();
        (void)result;
        return std::chrono::duration<double, std::micro>(t1 - t0).count();
    }
};

// ── 策略 B：const std::string&（旧写法）─────────────────────────
// 注意：调用方传入 string 字面量或 char* 时会触发隐式 string 构造
class ConstStringRefBench : public IStringBench {
public:
    std::string strategyName() const override {
        return "const std::string&";
    }

    static size_t countChars(const std::string& s) {
        size_t n = 0;
        for (char c : s) n += (c != ' ');
        return n;
    }

    double benchmark(const std::string& data, int repeat) const override {
        auto t0 = std::chrono::high_resolution_clock::now();
        volatile size_t result = 0;
        for (int i = 0; i < repeat; ++i)
            result = countChars(data);   // data 已是 string，无额外分配
        auto t1 = std::chrono::high_resolution_clock::now();
        (void)result;
        return std::chrono::duration<double, std::micro>(t1 - t0).count();
    }
};

// ── 策略 C：const char*（C 风格，丢失长度信息）──────────────────
class ConstCharPtrBench : public IStringBench {
public:
    std::string strategyName() const override {
        return "const char*";
    }

    static size_t countChars(const char* s) {
        size_t n = 0;
        while (*s) { n += (*s != ' '); ++s; }   // 需要遍历找 '\0'
        return n;
    }

    double benchmark(const std::string& data, int repeat) const override {
        auto t0 = std::chrono::high_resolution_clock::now();
        volatile size_t result = 0;
        for (int i = 0; i < repeat; ++i)
            result = countChars(data.c_str());
        auto t1 = std::chrono::high_resolution_clock::now();
        (void)result;
        return std::chrono::duration<double, std::micro>(t1 - t0).count();
    }
};

// ════════════════════════════════════════════════════════════════
// BenchmarkRunner（Context）
// ════════════════════════════════════════════════════════════════
void BenchmarkRunner::add(std::unique_ptr<IStringBench> s) {
    strategies_.push_back(std::move(s));
}

void BenchmarkRunner::run(const std::string& data, int repeat) const {
    printf("数据长度: %zu 字节  重复次数: %d\n\n", data.size(), repeat);
    printf("  %-26s %12s\n", "策略", "耗时(μs)");
    printf("  %-26s %12s\n", "──────────────────────────", "────────────");

    for (const auto& s : strategies_) {
        double us = s->benchmark(data, repeat);
        printf("  %-26s %12.1f\n", s->strategyName().c_str(), us);
    }

    printf("\n提示：string_view 传参在接收字面量/char* 时比 const string& 少一次堆分配。\n");
}

// ════════════════════════════════════════════════════════════════
// DemoPerf
// ════════════════════════════════════════════════════════════════
std::string DemoPerf::name() const { return "性能对比（Strategy 模式）"; }

std::string DemoPerf::description() const {
    return "IStringBench 策略接口 + 三种具体策略："
           "string_view / const string& / const char*；"
           "BenchmarkRunner 统一计时并打印对比结果。";
}

void DemoPerf::run() {
    // 构造一段足够长的测试字符串
    std::string data(100'000, 'x');
    for (size_t i = 10; i < data.size(); i += 10) data[i] = ' ';

    BenchmarkRunner runner;
    runner.add(std::make_unique<StringViewBench>());
    runner.add(std::make_unique<ConstStringRefBench>());
    runner.add(std::make_unique<ConstCharPtrBench>());
    runner.run(data, 1000);
}
