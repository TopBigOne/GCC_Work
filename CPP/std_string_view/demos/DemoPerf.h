#pragma once
/**
 * DemoPerf.h — 模块3：性能对比
 * 设计模式：Strategy（策略模式）
 *
 * 定义 IStringBench 策略接口，三种具体策略分别以：
 *   - std::string_view    （零拷贝，现代推荐写法）
 *   - const std::string&  （旧写法，传入字面量时触发堆分配）
 *   - const char*         （C 风格，丢失长度信息）
 * 作为函数参数接收字符串，通过 BenchmarkRunner 统一计时对比。
 */
#include "IDemo.h"
#include <string>
#include <memory>
#include <vector>

// ── Strategy 接口 ──────────────────────────────────────────────
class IStringBench {
public:
    virtual ~IStringBench() = default;
    virtual std::string strategyName() const = 0;
    // 对 data 执行 repeat 次字符统计，返回耗时（微秒）
    virtual double benchmark(const std::string& data, int repeat) const = 0;
};

// ── Context：统一执行并打印结果 ────────────────────────────────
class BenchmarkRunner {
public:
    void add(std::unique_ptr<IStringBench> strategy);
    void run(const std::string& data, int repeat) const;
private:
    std::vector<std::unique_ptr<IStringBench>> strategies_;
};

// ── Demo 入口 ──────────────────────────────────────────────────
class DemoPerf : public IDemo {
public:
    std::string name()        const override;
    std::string description() const override;
    void        run()               override;
};
