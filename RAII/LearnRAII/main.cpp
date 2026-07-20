#include <iostream>

#include "resource_guard/resource_guard.h"    // 模块1：手写 RAII 基础 + unique_ptr 对比
#include "factory_resource/factory_resource.h"  // 模块2：工厂模式
#include "proxy_resource/proxy_resource.h"    // 模块3：代理模式
#include "crtp_policy/crtp_policy.h"          // 模块4：CRTP + Policy
#include "scope_guard/scope_guard.h"          // 模块5：ScopeGuard

// ============================================================
// RAII 学习项目入口
//
// 知识点总览：
//   1. resource_guard  — RAII 核心：构造获取，析构释放，禁止拷贝，允许移动
//   2. factory_resource— 工厂模式：抽象接口 + unique_ptr 管理多态资源
//   3. proxy_resource  — 代理模式：日志代理、懒加载代理、访问控制代理
//   4. crtp_policy     — CRTP：编译期多态，策略注入，零虚函数开销
//   5. scope_guard     — ScopeGuard：通用 defer，dismiss 取消，异常感知变体
// ============================================================

static void print_separator(int module) {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║           RAII 模块 " << module << " 演示                ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "        Learn RAII — C++20 示例          \n";
    std::cout << "========================================\n";

    // ---- 模块1：手写 RAII 基础 ----
    print_separator(1);
    raii::demo_resource_guard();

    // ---- 模块2：工厂模式 ----
    print_separator(2);
    raii::factory::demo_factory_resource();

    // ---- 模块3：代理模式 ----
    print_separator(3);
    raii::proxy::demo_proxy_resource();

    // ---- 模块4：CRTP + Policy ----
    print_separator(4);
    raii::crtp::demo_crtp_policy();

    // ---- 模块5：ScopeGuard ----
    print_separator(5);
    raii::scope::demo_scope_guard();

    std::cout << "\n========================================\n";
    std::cout << "            所有模块演示完毕              \n";
    std::cout << "========================================\n";

    return 0;
}
