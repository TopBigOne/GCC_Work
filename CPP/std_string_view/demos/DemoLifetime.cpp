#include "DemoLifetime.h"
#include <string_view>
#include <string>
#include <cstdio>

std::string DemoLifetime::name() const { return "生命周期陷阱"; }

std::string DemoLifetime::description() const {
    return "string_view 不持有内存，不延长引用对象的生命周期；"
           "演示安全用法、常见陷阱（注释展示）、安全转换方式。";
}

// ── 辅助：安全地从函数返回 string_view ───────────────────────────
// 正确：返回指向入参的 view（入参生命周期由调用方保证）
static std::string_view safeSubstr(std::string_view s, size_t pos, size_t len) {
    if (pos >= s.size()) return {};
    return s.substr(pos, len);
}

// 错误示范（❌ 不要这样写）：
// static std::string_view dangerousReturn() {
//     std::string local = "i am local";
//     return local;   // local 析构后 view 悬空 → UB
// }

// ── 辅助：展示"接受 string_view 参数"的正确函数签名 ──────────────
static void printLength(std::string_view sv) {
    printf("  字符串长度: %zu\n", sv.size());
}

void DemoLifetime::run() {

    // ══ 1. 安全：view 的生命周期 < 原始数据生命周期 ══════════════
    printf("--- [安全] view 生命周期 < 原始数据 ---\n");
    {
        std::string owner = "I own this memory";
        std::string_view sv = owner;        // sv 指向 owner 的内部缓冲区
        printf("  sv = \"%.*s\"\n", (int)sv.size(), sv.data());
        // owner 在整个块内存活，sv 始终有效
    }   // owner 和 sv 同时离开作用域，安全

    // ══ 2. 陷阱：将临时 string 赋给 string_view ════════════════════
    printf("\n--- [陷阱] 临时 string → string_view（注释展示）---\n");
    // ❌ 下面的代码会产生悬空引用（dangling view），行为未定义：
    //
    //   std::string_view danger = std::string("temporary");
    //   // std::string("temporary") 是临时对象，该行结束后即析构
    //   // danger.data() 此时指向已释放的内存 → UB
    //   printf("%.*s\n", (int)danger.size(), danger.data());
    //
    printf("  （危险代码已注释，不实际执行；用 AddressSanitizer 可检测此类 UB）\n");

    // ══ 3. 陷阱：string 被 resize/修改后 view 失效 ════════════════
    printf("\n--- [陷阱] string 重新分配后 view 失效 ---\n");
    // ❌ 下面是错误示范：
    //
    //   std::string s = "hello";
    //   std::string_view sv = s;
    //   s += " world";          // 可能触发 realloc，sv.data() 失效
    //   printf("%.*s\n", (int)sv.size(), sv.data());  // UB
    //
    printf("  （危险代码已注释；修改 string 后切勿继续使用已有的 view）\n");

    // ══ 4. 安全：string_view → std::string（显式构造） ════════════
    printf("\n--- [安全] string_view 转回 std::string ---\n");
    std::string_view sv = "need a copy";
    std::string owned(sv);                  // 显式拷贝，拥有独立内存
    owned += "!";
    printf("  string_view: \"%.*s\"\n", (int)sv.size(), sv.data());
    printf("  std::string: \"%s\"\n", owned.c_str());

    // ══ 5. 安全：函数参数使用 string_view ════════════════════════
    printf("\n--- [安全] 函数参数使用 string_view ---\n");
    printLength("literal");               // 字面量：无堆分配
    printLength(std::string("owned"));    // std::string：无额外拷贝
    printLength(safeSubstr("hello world", 6, 5));   // 子串：零拷贝

    // ══ 6. 安全：在结构体/容器中存储 string_view 的注意事项 ══════
    printf("\n--- [注意] 结构体中存储 string_view ---\n");
    printf("  若结构体中含 string_view 成员，确保原始字符串生命周期\n"
           "  长于该结构体实例，否则同样会产生悬空引用。\n");
}
