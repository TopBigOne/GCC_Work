#pragma once
/**
 * DemoPractical.h — 模块5：实战场景
 * 设计模式：Facade（外观模式）
 *
 * StringParser 封装常见字符串处理原语，内部全程使用
 * string_view，对外提供简洁接口，避免不必要的字符串拷贝。
 *
 * 提供的功能：
 *   - split(sv, delim)        — 零拷贝分割
 *   - parseKV(sv, sep)        — key=value 解析
 *   - trim(sv)                — 去除首尾空白
 *   - parseRequestLine(sv)    — HTTP 请求行解析
 */
#include "IDemo.h"
#include <string_view>
#include <vector>
#include <optional>

// ── Facade ─────────────────────────────────────────────────────
class StringParser {
public:
    static std::vector<std::string_view>
    split(std::string_view s, char delim);

    static std::pair<std::string_view, std::string_view>
    parseKV(std::string_view s, char sep = '=');

    static std::string_view
    trim(std::string_view s);

    struct HttpRequestLine {
        std::string_view method;
        std::string_view path;
        std::string_view version;
    };

    static std::optional<HttpRequestLine>
    parseRequestLine(std::string_view line);
};

// ── Demo 入口 ──────────────────────────────────────────────────
class DemoPractical : public IDemo {
public:
    std::string name()        const override;
    std::string description() const override;
    void        run()               override;
};
