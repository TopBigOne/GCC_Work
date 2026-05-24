#include "proxy_resource.h"
#include <iostream>

namespace raii {
namespace proxy {

// ---- DatabaseConnection ----

DatabaseConnection::DatabaseConnection(const std::string& url) : url_(url) {
    std::cout << "    [DB] 真实连接建立: " << url << "\n";
}

DatabaseConnection::~DatabaseConnection() {
    std::cout << "    [DB] 连接关闭: " << url_ << "\n";
}

void DatabaseConnection::query(const std::string& sql) {
    std::cout << "    [DB] 执行 SQL: " << sql << "\n";
}

const std::string& DatabaseConnection::url() const { return url_; }

// ---- demo ----

void demo_proxy_resource() {
    std::cout << "\n====== 模块3：代理模式 + RAII ======\n";

    std::cout << "\n[日志代理 LoggingProxy]\n";
    {
        LoggingProxy<DatabaseConnection> proxy("mysql://localhost/mydb");
        proxy->query("SELECT * FROM users");
        proxy->query("UPDATE orders SET status='done'");
    }

    std::cout << "\n[懒加载代理 LazyProxy]\n";
    {
        LazyProxy<DatabaseConnection> lazy([]() {
            return std::make_unique<DatabaseConnection>("postgres://localhost/prod");
        });
        std::cout << "  代理已创建，初始化状态: "
                  << (lazy.is_initialized() ? "已初始化" : "未初始化") << "\n";
        lazy->query("SELECT count(*) FROM events");
        std::cout << "  初始化状态: "
                  << (lazy.is_initialized() ? "已初始化" : "未初始化") << "\n";
    }

    std::cout << "\n[访问控制代理 GuardedProxy]\n";
    {
        bool has_permission = true;
        GuardedProxy<DatabaseConnection> guarded(
            std::make_unique<DatabaseConnection>("sqlite:///local.db"),
            [&has_permission] { return has_permission; }
        );
        guarded->query("INSERT INTO logs VALUES (...)");
        has_permission = false;
        try {
            guarded->query("DROP TABLE users");
        } catch (const std::exception& e) {
            std::cout << "  捕获异常: " << e.what() << "\n";
        }
    }

    std::cout << "====== 模块3 结束 ======\n";
}

}  // namespace proxy
}  // namespace raii
