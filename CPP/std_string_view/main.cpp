/**
 * main.cpp — std::string_view 学习 demos 入口
 *
 * 设计模式总览：
 *   Template Method — IDemo：统一接口，main 通过多态批量驱动所有 demo
 *   Strategy        — DemoPerf：IStringBench 接口 + 三种具体策略
 *   Facade          — DemoPractical：StringParser 封装常用字符串解析原语
 *
 * 文件结构：
 *   demos/
 *     IDemo.h              Template Method 接口
 *     DemoBasic            模块1：string_view 本质与构造方式
 *     DemoOperations       模块2：substr/find/starts_with 等常用操作
 *     DemoPerf             模块3：Strategy 模式性能对比
 *     DemoLifetime         模块4：生命周期陷阱与安全用法
 *     DemoPractical        模块5：StringParser Facade 实战场景
 */
#include "demos/DemoBasic.h"
#include "demos/DemoLifetime.h"
#include "demos/DemoOperations.h"
#include "demos/DemoPerf.h"
#include "demos/DemoPractical.h"
#include "demos/IDemo.h"

#include <cstdio>
#include <memory>
#include <vector>

static void printHeader(const IDemo& demo, int idx) {
    printf("\n");
    printf("══════════════════════════════════════════════════════════\n");
    printf("  Demo %d: %s\n", idx, demo.name().c_str());
    printf("  %s\n", demo.description().c_str());
    printf("──────────────────────────────────────────────────────────\n");
}

int main() {
    std::vector<std::unique_ptr<IDemo>> demos;
    demos.push_back(std::make_unique<DemoBasic>());
    demos.push_back(std::make_unique<DemoOperations>());
    demos.push_back(std::make_unique<DemoPerf>());
    demos.push_back(std::make_unique<DemoLifetime>());
    demos.push_back(std::make_unique<DemoPractical>());

    printf("std::string_view 学习 demos — C++20\n");

    int idx = 1;
    for (auto& demo : demos) {
        printHeader(*demo, idx++);
        try {
            demo->run();
        } catch (const std::exception& e) {
            printf("  [ERROR] %s\n", e.what());
        }
    }

    printf("\n══════════════════════════════════════════════════════════\n");
    printf("  全部 demo 运行完毕\n");
    printf("══════════════════════════════════════════════════════════\n\n");
    return 0;
}
