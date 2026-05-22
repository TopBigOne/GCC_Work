#include "DemoBasic.h"
#include <string_view>
#include <string>
#include <cstdio>

std::string DemoBasic::name() const {
    return "基本概念与构造";
}

std::string DemoBasic::description() const {
    return "string_view 的本质：const char* + size_t，不持有内存；"
           "演示各种构造方式与基本属性访问。";
}

void DemoBasic::run() {
    // ── 1. 从字符串字面量构造（零开销，指向静态存储区）──────────
    std::string_view sv1 = "hello, string_view";
    printf("[1] 字面量构造         : \"%.*s\"  size=%zu\n", static_cast<int>(sv1.size()), sv1.data(), sv1.size());

    // ── 2. 从 std::string 构造（隐式转换，无拷贝）────────────────
    std::string str = "来自 std::string";
    std::string_view sv2 = str;
    printf("[2] std::string 构造  : \"%.*s\"  size=%zu\n", static_cast<int>(sv2.size()), sv2.data(), sv2.size());

    // ── 3. 从 char[] 构造 ─────────────────────────────────────────
    char buf[] = "char array source";
    std::string_view sv3 = buf;
    printf("[3] char[] 构造       : \"%.*s\"  size=%zu\n",(int)sv3.size(), sv3.data(), sv3.size());

    // ── 4. 带长度构造：只取前 N 个字节 ───────────────────────────
    std::string_view sv4(str.data(), 6);    // 前6字节
    printf("[4] 带长度构造(前6字节) : \"%.*s\"\n",(int)sv4.size(), sv4.data());

    // ── 5. 基本属性 ───────────────────────────────────────────────
    printf("\n--- 基本属性 ---\n");
    printf("size=%zu  length=%zu  empty=%s\n",
           sv1.size(), sv1.length(), sv1.empty() ? "true" : "false");
    printf("front='%c'  back='%c'  sv1[0]='%c'\n",
           sv1.front(), sv1.back(), sv1[0]);

    // ── 6. data() 不保证 null-terminated ─────────────────────────
    // 用 char[] 不含 '\0' 来模拟非 null-terminated 场景
    const char raw[] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
    std::string_view sv5(raw, 5);   // 只看 "hello"，后面没有 '\0'

    printf("\n--- data() 不保证 null-terminated ---\n");
    // 错误示范（注释掉）：printf("%s", sv5.data()); // UB：越界读
    // 正确：用 %.*s 指定长度
    printf("安全打印: \"%.*s\"\n", (int)sv5.size(), sv5.data());

    // ── 7. 相等性比较（不拷贝，逐字节比较）──────────────────────
    std::string_view a = "abc";
    std::string_view b = "abc";
    printf("\n--- 比较 ---\n");
    printf("\"abc\" == \"abc\": %s\n", (a == b) ? "true" : "false");
    printf("\"abc\"  < \"abd\": %s\n", (a  < "abd") ? "true" : "false");
}
