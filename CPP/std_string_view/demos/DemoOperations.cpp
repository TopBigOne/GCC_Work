#include "DemoOperations.h"
#include <string_view>
#include <cstdio>

std::string DemoOperations::name() const {
    return "常用操作";
}

std::string DemoOperations::description() const {
    return "substr/find/rfind/starts_with/ends_with/"
           "remove_prefix/remove_suffix/迭代器遍历";
}

// 辅助：打印 string_view（%.*s 方式）
static void print_sv(const char* label, std::string_view sv) {
    printf("%-30s \"%.*s\"\n", label, (int)sv.size(), sv.data());
}

void DemoOperations::run() {
    std::string_view sv = "  Hello, std::string_view!  ";

    // ── 1. substr：零拷贝，返回新 string_view ────────────────────
    printf("--- substr（零拷贝）---\n");
    std::string_view sub = sv.substr(2, 5);   // "Hello"
    print_sv("sv.substr(2, 5):", sub);

    // ── 2. find / rfind ───────────────────────────────────────────
    printf("\n--- find / rfind ---\n");
    size_t pos = sv.find('s');
    printf("sv.find('s')          = %zu\n", pos);

    size_t rpos = sv.rfind('!');
    printf("sv.rfind('!')         = %zu\n", rpos);

    size_t npos = sv.find("xyz");
    printf("sv.find(\"xyz\") npos? = %s\n",
           (npos == std::string_view::npos) ? "yes" : "no");

    // ── 3. find_first_of / find_last_of ──────────────────────────
    printf("\n--- find_first_of / find_last_of ---\n");
    size_t first_vowel = sv.find_first_of("aeiouAEIOU");
    printf("find_first_of vowels  pos=%zu  char='%c'\n",
           first_vowel, sv[first_vowel]);

    size_t last_space = sv.find_last_of(' ');
    printf("find_last_of(' ')     pos=%zu\n", last_space);

    // ── 4. starts_with / ends_with（C++20）───────────────────────
    printf("\n--- starts_with / ends_with (C++20) ---\n");
    std::string_view trimmed = sv.substr(2, sv.size() - 4);   // 去掉两端空格
    print_sv("trimmed:", trimmed);
    printf("starts_with(\"Hello\") = %s\n",
           trimmed.starts_with("Hello") ? "true" : "false");
    printf("ends_with(\"!\")       = %s\n",
           trimmed.ends_with("!") ? "true" : "false");

    // ── 5. remove_prefix / remove_suffix（原地修改视图范围）─────
    printf("\n--- remove_prefix / remove_suffix ---\n");
    std::string_view sv2 = ">>>target<<<";
    print_sv("原始:", sv2);
    sv2.remove_prefix(3);
    print_sv("remove_prefix(3):", sv2);
    sv2.remove_suffix(3);
    print_sv("remove_suffix(3):", sv2);

    // ── 6. 范围 for 遍历 ─────────────────────────────────────────
    printf("\n--- 迭代器遍历（统计字母个数）---\n");
    std::string_view text = "Hello, World!";
    int letter_count = 0;
    for (char c : text) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            ++letter_count;
    }
    printf("\"%.*s\" 中字母个数 = %d\n",
           (int)text.size(), text.data(), letter_count);
}
