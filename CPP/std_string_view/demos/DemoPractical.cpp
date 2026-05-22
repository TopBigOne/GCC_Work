#include "DemoPractical.h"
#include <string_view>
#include <string>
#include <vector>
#include <optional>
#include <cstdio>

// ════════════════════════════════════════════════════════════════
// StringParser（Facade 模式）
// 封装常用字符串处理原语，内部全程使用 string_view，零拷贝。
// ════════════════════════════════════════════════════════════════

std::vector<std::string_view> StringParser::split(std::string_view s, char delim) {
    std::vector<std::string_view> result;
    size_t start = 0;
    while (true) {
        size_t pos = s.find(delim, start);
        if (pos == std::string_view::npos) {
            result.push_back(s.substr(start));
            break;
        }
        result.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
    return result;
}

std::pair<std::string_view, std::string_view>
StringParser::parseKV(std::string_view s, char sep) {
    size_t pos = s.find(sep);
    if (pos == std::string_view::npos)
        return {s, {}};
    return {s.substr(0, pos), s.substr(pos + 1)};
}

std::string_view StringParser::trim(std::string_view s) {
    const std::string_view ws = " \t\r\n";
    size_t begin = s.find_first_not_of(ws);
    if (begin == std::string_view::npos) return {};
    size_t end = s.find_last_not_of(ws);
    return s.substr(begin, end - begin + 1);
}

std::optional<StringParser::HttpRequestLine>
StringParser::parseRequestLine(std::string_view line) {
    // 格式：METHOD SP path SP HTTP/version
    size_t sp1 = line.find(' ');
    if (sp1 == std::string_view::npos) return std::nullopt;

    size_t sp2 = line.find(' ', sp1 + 1);
    if (sp2 == std::string_view::npos) return std::nullopt;

    return HttpRequestLine{
        .method  = line.substr(0, sp1),
        .path    = line.substr(sp1 + 1, sp2 - sp1 - 1),
        .version = line.substr(sp2 + 1),
    };
}

// ════════════════════════════════════════════════════════════════
// DemoPractical
// ════════════════════════════════════════════════════════════════
std::string DemoPractical::name() const { return "实战场景（Facade 模式）"; }

std::string DemoPractical::description() const {
    return "StringParser Facade 封装 split/parseKV/trim/parseRequestLine，"
           "内部全程 string_view 零拷贝，演示常见解析场景。";
}

void DemoPractical::run() {

    // ── 1. split：CSV 行解析 ──────────────────────────────────────
    printf("--- split：CSV 解析 ---\n");
    std::string_view csv = "name,age,city,score";
    auto fields = StringParser::split(csv, ',');
    printf("原始: \"%.*s\"\n", (int)csv.size(), csv.data());
    printf("分割结果（%zu 个字段）:\n", fields.size());
    for (size_t i = 0; i < fields.size(); ++i) {
        printf("  [%zu] \"%.*s\"\n", i, (int)fields[i].size(), fields[i].data());
    }

    // ── 2. parseKV：配置文件解析 ─────────────────────────────────
    printf("\n--- parseKV：配置文件解析 ---\n");
    const char* configs[] = {
        "host=localhost",
        "port=8080",
        "timeout=30",
        "debug=true",
    };
    for (const char* line : configs) {
        auto [key, val] = StringParser::parseKV(line);
        printf("  key=%-12.*s val=%.*s\n",
               (int)key.size(), key.data(),
               (int)val.size(), val.data());
    }

    // ── 3. trim：去除首尾空白 ─────────────────────────────────────
    printf("\n--- trim ---\n");
    std::string_view padded = "   \t  trimme this!  \n  ";
    std::string_view trimmed = StringParser::trim(padded);
    printf("原始:  |%.*s|\n", (int)padded.size(),  padded.data());
    printf("trim后:|%.*s|\n", (int)trimmed.size(), trimmed.data());

    // ── 4. HTTP 请求行解析 ────────────────────────────────────────
    printf("\n--- HTTP 请求行解析 ---\n");
    const char* requests[] = {
        "GET /index.html HTTP/1.1",
        "POST /api/login HTTP/2",
        "DELETE /resource/123 HTTP/1.1",
        "INVALID_LINE",              // 格式错误，返回 nullopt
    };
    for (const char* req : requests) {
        std::string_view line = req;
        auto result = StringParser::parseRequestLine(line);
        if (result) {
            printf("  %-38.*s → method=%-8.*s path=%-20.*s ver=%.*s\n",
                   (int)line.size(),            line.data(),
                   (int)result->method.size(),  result->method.data(),
                   (int)result->path.size(),    result->path.data(),
                   (int)result->version.size(), result->version.data());
        } else {
            printf("  %-38.*s → 解析失败\n", (int)line.size(), line.data());
        }
    }

    // ── 5. 组合使用：解析 URL query string ───────────────────────
    printf("\n--- 组合：解析 URL query string ---\n");
    std::string_view query = "name=Alice&age=30&city=Shanghai&lang=zh";
    printf("query: \"%.*s\"\n", (int)query.size(), query.data());
    auto pairs = StringParser::split(query, '&');
    for (auto& pair : pairs) {
        auto [k, v] = StringParser::parseKV(pair);
        printf("  %-10.*s = %.*s\n",
               (int)k.size(), k.data(),
               (int)v.size(), v.data());
    }
}
