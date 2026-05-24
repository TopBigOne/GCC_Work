# Header/Implementation Split — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将每个模块的非模板实现从 `.h` 移至对应 `.cpp`，头文件只保留声明和模板定义。

**Architecture:** 模板类/函数必须留在头文件（编译器需在实例化点看到完整定义）；非模板类方法、`demo_*` 函数、具体资源类的实现全部移至 `.cpp`。Policy 结构体和 CRTP 基类保留在头文件（作为模板参数必须可见）。

**Tech Stack:** C++20, CMake 3.16+, AppleClang 17

---

## 文件划分

| 头文件（声明 + 模板） | 实现文件（非模板实现） |
|---|---|
| `resource_guard.h` | `resource_guard.cpp` |
| `factory_resource.h` | `factory_resource.cpp` |
| `proxy_resource.h` | `proxy_resource.cpp` |
| `crtp_policy.h` | `crtp_policy.cpp` |
| `scope_guard.h` | `scope_guard.cpp` |
| `main.cpp` | — （不变） |
| `CMakeLists.txt` | — （新增 5 个 .cpp） |

**模板必须留在头文件的类：**
- `MemoryGuard<T>`（resource_guard.h）
- `LoggingProxy<T>`, `LazyProxy<T>`, `GuardedProxy<T>`（proxy_resource.h）
- `ResourceBase<Derived,Policy>`（crtp_policy.h）
- `make_scope_guard<F>`, `detail::operator+`（scope_guard.h）

---

### Task 1: resource_guard.h / resource_guard.cpp

**Files:**
- Modify: `resource_guard.h`
- Create: `resource_guard.cpp`

- [ ] **Step 1: 重写 resource_guard.h — 只保留声明 + MemoryGuard 模板**

```cpp
#pragma once
#include <cstdio>
#include <cstddef>
#include <string>
#include <typeinfo>

namespace raii {

class FileGuard {
public:
    explicit FileGuard(const std::string& path, const char* mode = "w+");
    ~FileGuard() noexcept;
    FileGuard(const FileGuard&)            = delete;
    FileGuard& operator=(const FileGuard&) = delete;
    FileGuard(FileGuard&& other) noexcept;
    FileGuard& operator=(FileGuard&& other) noexcept;
    FILE* get() const noexcept;
    bool  is_open() const noexcept;
    void  write(const std::string& text);
private:
    FILE* file_;
};

// MemoryGuard 是模板，必须保留在头文件
template <typename T>
class MemoryGuard {
public:
    explicit MemoryGuard(std::size_t size)
        : ptr_(new T[size]()), size_(size) {
        // 打印放 cpp 中会导致 typeid 不可用；此处保留 inline
        std::cout << "[MemoryGuard] 分配 " << size
                  << " 个 " << typeid(T).name() << "\n";
    }
    ~MemoryGuard() noexcept {
        delete[] ptr_;
        std::cout << "[MemoryGuard] 释放内存\n";
    }
    MemoryGuard(const MemoryGuard&)            = delete;
    MemoryGuard& operator=(const MemoryGuard&) = delete;
    MemoryGuard(MemoryGuard&& other) noexcept
        : ptr_(other.ptr_), size_(other.size_) {
        other.ptr_  = nullptr;
        other.size_ = 0;
    }
    T&          operator[](std::size_t i) { return ptr_[i]; }
    const T&    operator[](std::size_t i) const { return ptr_[i]; }
    T*          get() const noexcept { return ptr_; }
    std::size_t size() const noexcept { return size_; }
private:
    T*          ptr_;
    std::size_t size_;
};

void demo_resource_guard();

}  // namespace raii
```

- [ ] **Step 2: 创建 resource_guard.cpp — FileGuard 实现 + demo**

```cpp
#include "resource_guard.h"
#include <iostream>
#include <memory>
#include <stdexcept>

namespace raii {

FileGuard::FileGuard(const std::string& path, const char* mode)
    : file_(std::fopen(path.c_str(), mode)) {
    if (!file_) throw std::runtime_error("FileGuard: 无法打开文件: " + path);
    std::cout << "[FileGuard] 打开文件: " << path << "\n";
}

FileGuard::~FileGuard() noexcept {
    if (file_) {
        std::fclose(file_);
        std::cout << "[FileGuard] 自动关闭文件\n";
    }
}

FileGuard::FileGuard(FileGuard&& other) noexcept : file_(other.file_) {
    other.file_ = nullptr;
    std::cout << "[FileGuard] 移动构造\n";
}

FileGuard& FileGuard::operator=(FileGuard&& other) noexcept {
    if (this != &other) {
        if (file_) std::fclose(file_);
        file_       = other.file_;
        other.file_ = nullptr;
    }
    return *this;
}

FILE* FileGuard::get() const noexcept { return file_; }
bool  FileGuard::is_open() const noexcept { return file_ != nullptr; }
void  FileGuard::write(const std::string& text) {
    if (file_) std::fputs(text.c_str(), file_);
}

void demo_resource_guard() {
    std::cout << "\n====== 模块1：RAII 基础 ======\n";

    std::cout << "\n[手写 FileGuard]\n";
    {
        FileGuard f("/tmp/raii_test.txt");
        f.write("Hello RAII\n");
        std::cout << "  文件已写入，离开作用域时自动关闭...\n";
    }

    std::cout << "\n[手写 MemoryGuard]\n";
    {
        MemoryGuard<int> buf(8);
        for (std::size_t i = 0; i < buf.size(); ++i) buf[i] = static_cast<int>(i * i);
        std::cout << "  buf[3] = " << buf[3] << "\n";
    }

    std::cout << "\n[std::unique_ptr 对比]\n";
    {
        auto file_deleter = [](FILE* f) {
            if (f) { std::fclose(f); std::cout << "[unique_ptr] 自动关闭 FILE*\n"; }
        };
        std::unique_ptr<FILE, decltype(file_deleter)>
            uf(std::fopen("/tmp/raii_unique.txt", "w+"), file_deleter);
        if (uf) std::fputs("unique_ptr file\n", uf.get());

        std::unique_ptr<int[]> arr(new int[8]{});
        arr[3] = 42;
        std::cout << "  arr[3] = " << arr[3] << "\n";
    }

    std::cout << "\n[异常安全演示]\n";
    try {
        MemoryGuard<double> buf(4);
        buf[0] = 3.14;
        throw std::runtime_error("模拟异常");
    } catch (const std::exception& e) {
        std::cout << "  捕获异常: " << e.what() << "（内存已在异常前自动释放）\n";
    }

    std::cout << "====== 模块1 结束 ======\n";
}

}  // namespace raii
```

---

### Task 2: factory_resource.h / factory_resource.cpp

**Files:**
- Modify: `factory_resource.h`
- Create: `factory_resource.cpp`

- [ ] **Step 1: 重写 factory_resource.h — 只保留声明**

```cpp
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdio>

namespace raii {
namespace factory {

class IResource {
public:
    virtual ~IResource() = default;
    virtual void        use()                     = 0;
    virtual std::string name() const              = 0;
    virtual bool        is_valid() const noexcept = 0;
};

class MemoryResource final : public IResource {
public:
    explicit MemoryResource(std::size_t size);
    ~MemoryResource() override;
    void        use() override;
    std::string name() const override;
    bool        is_valid() const noexcept override;
private:
    std::vector<uint8_t> buffer_;
    std::string          name_;
};

class TempFileResource final : public IResource {
public:
    explicit TempFileResource(const std::string& path);
    ~TempFileResource() override;
    TempFileResource(const TempFileResource&)            = delete;
    TempFileResource& operator=(const TempFileResource&) = delete;
    void        use() override;
    std::string name() const override;
    bool        is_valid() const noexcept override;
private:
    std::string path_;
    FILE*       file_;
};

class NetworkResource final : public IResource {
public:
    explicit NetworkResource(const std::string& host, int port);
    ~NetworkResource() override;
    NetworkResource(const NetworkResource&)            = delete;
    NetworkResource& operator=(const NetworkResource&) = delete;
    void        use() override;
    std::string name() const override;
    bool        is_valid() const noexcept override;
private:
    std::string host_;
    int         port_;
    bool        connected_;
};

class ResourceFactory {
public:
    ResourceFactory() = delete;
    static std::unique_ptr<IResource> create_memory(std::size_t size);
    static std::unique_ptr<IResource> create_temp_file(const std::string& path);
    static std::unique_ptr<IResource> create_network(const std::string& host, int port);
    static std::vector<std::unique_ptr<IResource>> create_all();
};

void demo_factory_resource();

}  // namespace factory
}  // namespace raii
```

- [ ] **Step 2: 创建 factory_resource.cpp — 所有实现**

```cpp
#include "factory_resource.h"
#include <iostream>
#include <stdexcept>

namespace raii {
namespace factory {

// ---- MemoryResource ----
MemoryResource::MemoryResource(std::size_t size)
    : buffer_(size, 0), name_("MemoryResource[" + std::to_string(size) + "]") {
    std::cout << "  [" << name_ << "] 构造 — 分配 " << size << " 字节\n";
}
MemoryResource::~MemoryResource() {
    std::cout << "  [" << name_ << "] 析构 — 释放内存\n";
}
void MemoryResource::use() {
    buffer_[0] = 42;
    std::cout << "  [" << name_ << "] 使用中，buffer[0]="
              << static_cast<int>(buffer_[0]) << "\n";
}
std::string MemoryResource::name() const { return name_; }
bool        MemoryResource::is_valid() const noexcept { return !buffer_.empty(); }

// ---- TempFileResource ----
TempFileResource::TempFileResource(const std::string& path)
    : path_(path), file_(std::fopen(path.c_str(), "w+")) {
    if (!file_) throw std::runtime_error("TempFileResource: 无法创建: " + path);
    std::cout << "  [TempFileResource] 构造 — 创建 " << path << "\n";
}
TempFileResource::~TempFileResource() {
    if (file_) {
        std::fclose(file_);
        std::remove(path_.c_str());
        std::cout << "  [TempFileResource] 析构 — 关闭并删除 " << path_ << "\n";
    }
}
void        TempFileResource::use() {
    std::fputs("factory resource test\n", file_);
    std::cout << "  [TempFileResource] 写入数据\n";
}
std::string TempFileResource::name() const { return "TempFileResource[" + path_ + "]"; }
bool        TempFileResource::is_valid() const noexcept { return file_ != nullptr; }

// ---- NetworkResource ----
NetworkResource::NetworkResource(const std::string& host, int port)
    : host_(host), port_(port), connected_(true) {
    std::cout << "  [NetworkResource] 构造 — 连接 " << host << ":" << port << "\n";
}
NetworkResource::~NetworkResource() {
    if (connected_) {
        connected_ = false;
        std::cout << "  [NetworkResource] 析构 — 断开连接 "
                  << host_ << ":" << port_ << "\n";
    }
}
void NetworkResource::use() {
    std::cout << "  [NetworkResource] 发送数据到 " << host_ << ":" << port_ << "\n";
}
std::string NetworkResource::name() const {
    return "NetworkResource[" + host_ + ":" + std::to_string(port_) + "]";
}
bool NetworkResource::is_valid() const noexcept { return connected_; }

// ---- ResourceFactory ----
std::unique_ptr<IResource> ResourceFactory::create_memory(std::size_t size) {
    return std::make_unique<MemoryResource>(size);
}
std::unique_ptr<IResource> ResourceFactory::create_temp_file(const std::string& path) {
    return std::make_unique<TempFileResource>(path);
}
std::unique_ptr<IResource> ResourceFactory::create_network(const std::string& host, int port) {
    return std::make_unique<NetworkResource>(host, port);
}
std::vector<std::unique_ptr<IResource>> ResourceFactory::create_all() {
    std::vector<std::unique_ptr<IResource>> resources;
    resources.push_back(create_memory(1024));
    resources.push_back(create_temp_file("/tmp/factory_raii.txt"));
    resources.push_back(create_network("127.0.0.1", 8080));
    return resources;
}

void demo_factory_resource() {
    std::cout << "\n====== 模块2：工厂模式 + RAII ======\n";

    std::cout << "\n[单独创建资源]\n";
    {
        auto mem = ResourceFactory::create_memory(256);
        auto net = ResourceFactory::create_network("localhost", 9090);
        mem->use();
        net->use();
        std::cout << "  离开作用域，资源自动释放...\n";
    }

    std::cout << "\n[批量创建，多态析构]\n";
    {
        auto resources = ResourceFactory::create_all();
        for (auto& r : resources) {
            if (r->is_valid()) r->use();
        }
        std::cout << "  vector 析构，所有资源自动释放（按逆序）...\n";
    }

    std::cout << "====== 模块2 结束 ======\n";
}

}  // namespace factory
}  // namespace raii
```

---

### Task 3: proxy_resource.h / proxy_resource.cpp

**Files:**
- Modify: `proxy_resource.h`
- Create: `proxy_resource.cpp`

- [ ] **Step 1: 重写 proxy_resource.h — DatabaseConnection 声明 + 模板类保留**

```cpp
#pragma once
#include <memory>
#include <string>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <iostream>

namespace raii {
namespace proxy {

// 非模板类：声明移此，实现到 .cpp
class DatabaseConnection {
public:
    explicit DatabaseConnection(const std::string& url);
    ~DatabaseConnection();
    DatabaseConnection(const DatabaseConnection&)            = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;
    void query(const std::string& sql);
    const std::string& url() const;
private:
    std::string url_;
};

// 模板类：必须保留在头文件
template <typename T>
class LoggingProxy {
public:
    template <typename... Args>
    explicit LoggingProxy(Args&&... args)
        : resource_(std::make_unique<T>(std::forward<Args>(args)...)) {
        log("LoggingProxy 构造完成");
    }
    ~LoggingProxy() { log("LoggingProxy 析构，释放资源"); }
    LoggingProxy(const LoggingProxy&)            = delete;
    LoggingProxy& operator=(const LoggingProxy&) = delete;
    T* operator->() { log("访问资源"); return resource_.get(); }
    T& operator*()  { log("解引用资源"); return *resource_; }
    T* get() noexcept { return resource_.get(); }
private:
    void log(const std::string& msg) const {
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        std::cout << "  [LogProxy|" << now % 100000 << "] " << msg << "\n";
    }
    std::unique_ptr<T> resource_;
};

template <typename T>
class LazyProxy {
public:
    using Factory = std::function<std::unique_ptr<T>()>;
    explicit LazyProxy(Factory factory) : factory_(std::move(factory)) {
        std::cout << "  [LazyProxy] 创建代理（资源尚未构造）\n";
    }
    ~LazyProxy() {
        if (resource_) std::cout << "  [LazyProxy] 析构，释放延迟创建的资源\n";
        else           std::cout << "  [LazyProxy] 析构，资源从未使用，无需释放\n";
    }
    LazyProxy(const LazyProxy&)            = delete;
    LazyProxy& operator=(const LazyProxy&) = delete;
    T* get() {
        if (!resource_) {
            std::cout << "  [LazyProxy] 首次访问，现在构造资源...\n";
            resource_ = factory_();
        }
        return resource_.get();
    }
    T* operator->() { return get(); }
    T& operator*()  { return *get(); }
    bool is_initialized() const noexcept { return resource_ != nullptr; }
private:
    Factory            factory_;
    std::unique_ptr<T> resource_;
};

template <typename T>
class GuardedProxy {
public:
    using Predicate = std::function<bool()>;
    GuardedProxy(std::unique_ptr<T> resource, Predicate can_access)
        : resource_(std::move(resource)), can_access_(std::move(can_access)) {
        std::cout << "  [GuardedProxy] 代理创建，资源受保护\n";
    }
    ~GuardedProxy() { std::cout << "  [GuardedProxy] 析构，资源释放\n"; }
    GuardedProxy(const GuardedProxy&)            = delete;
    GuardedProxy& operator=(const GuardedProxy&) = delete;
    T& operator*()  { check_access(); return *resource_; }
    T* operator->() { check_access(); return resource_.get(); }
private:
    void check_access() {
        if (!can_access_()) throw std::runtime_error("GuardedProxy: 访问被拒绝");
        std::cout << "  [GuardedProxy] 权限验证通过\n";
    }
    std::unique_ptr<T> resource_;
    Predicate          can_access_;
};

void demo_proxy_resource();

}  // namespace proxy
}  // namespace raii
```

- [ ] **Step 2: 创建 proxy_resource.cpp — DatabaseConnection 实现 + demo**

```cpp
#include "proxy_resource.h"
#include <iostream>

namespace raii {
namespace proxy {

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
```

---

### Task 4: crtp_policy.h / crtp_policy.cpp

**Files:**
- Modify: `crtp_policy.h`
- Create: `crtp_policy.cpp`

- [ ] **Step 1: 重写 crtp_policy.h — Policy/ResourceBase/具体类声明**

```cpp
#pragma once
#include <iostream>
#include <string>
#include <cstddef>
#include <cstdio>

namespace raii {
namespace crtp {

// Policy structs 必须留在头文件（作为模板参数）
struct SilentPolicy {
    static void on_acquire(const std::string&) {}
    static void on_release(const std::string&) {}
};

struct LogPolicy {
    static void on_acquire(const std::string& name) {
        std::cout << "  [LogPolicy] 获取资源: " << name << "\n";
    }
    static void on_release(const std::string& name) {
        std::cout << "  [LogPolicy] 释放资源: " << name << "\n";
    }
};

struct CountingPolicy {
    static inline std::size_t acquire_count = 0;
    static inline std::size_t release_count = 0;
    static void on_acquire(const std::string& name) {
        std::cout << "  [CountingPolicy] 获取 #" << ++acquire_count << ": " << name << "\n";
    }
    static void on_release(const std::string& name) {
        std::cout << "  [CountingPolicy] 释放 #" << ++release_count << ": " << name << "\n";
    }
    static void report() {
        std::cout << "  [CountingPolicy] 统计 — 获取: " << acquire_count
                  << ", 释放: " << release_count << "\n";
    }
};

// CRTP 基类必须留在头文件
template <typename Derived, typename Policy>
class ResourceBase {
public:
    explicit ResourceBase(std::string name) : name_(std::move(name)) {
        Policy::on_acquire(name_);
    }
    ~ResourceBase() {
        Policy::on_release(name_);
        static_cast<Derived&>(*this).do_release();
    }
    ResourceBase(const ResourceBase&)            = delete;
    ResourceBase& operator=(const ResourceBase&) = delete;
protected:
    const std::string& resource_name() const noexcept { return name_; }
private:
    std::string name_;
};

// 具体资源类：声明在头文件，实现在 .cpp
class LoggedBuffer : public ResourceBase<LoggedBuffer, LogPolicy> {
public:
    explicit LoggedBuffer(std::size_t size);
    void do_release();
    int& operator[](std::size_t i);
private:
    int*        data_;
    std::size_t size_;
};

class SilentFile : public ResourceBase<SilentFile, SilentPolicy> {
public:
    explicit SilentFile(const std::string& path);
    void do_release();
    void write(const char* s);
private:
    FILE*       file_;
    std::string path_;
};

class CountedSocket : public ResourceBase<CountedSocket, CountingPolicy> {
public:
    explicit CountedSocket(const std::string& endpoint);
    void do_release();
    void send(const std::string& data);
private:
    std::string endpoint_;
    bool        connected_;
};

void demo_crtp_policy();

}  // namespace crtp
}  // namespace raii
```

- [ ] **Step 2: 创建 crtp_policy.cpp — 具体资源类实现 + demo**

```cpp
#include "crtp_policy.h"
#include <iostream>

namespace raii {
namespace crtp {

// ---- LoggedBuffer ----
LoggedBuffer::LoggedBuffer(std::size_t size)
    : ResourceBase("LoggedBuffer[" + std::to_string(size) + "]")
    , data_(new int[size]())
    , size_(size) {
    std::cout << "    [LoggedBuffer] 分配 " << size << " 个 int\n";
}
void LoggedBuffer::do_release() {
    delete[] data_;
    data_ = nullptr;
    std::cout << "    [LoggedBuffer] 内存已释放\n";
}
int& LoggedBuffer::operator[](std::size_t i) { return data_[i]; }

// ---- SilentFile ----
SilentFile::SilentFile(const std::string& path)
    : ResourceBase("SilentFile[" + path + "]")
    , file_(std::fopen(path.c_str(), "w+"))
    , path_(path) {
    std::cout << "    [SilentFile] 打开 " << path << "\n";
}
void SilentFile::do_release() {
    if (file_) {
        std::fclose(file_);
        file_ = nullptr;
        std::cout << "    [SilentFile] 关闭文件（策略无日志）\n";
    }
}
void SilentFile::write(const char* s) { if (file_) std::fputs(s, file_); }

// ---- CountedSocket ----
CountedSocket::CountedSocket(const std::string& endpoint)
    : ResourceBase("CountedSocket[" + endpoint + "]")
    , endpoint_(endpoint)
    , connected_(true) {
    std::cout << "    [CountedSocket] 连接 " << endpoint << "\n";
}
void CountedSocket::do_release() {
    if (connected_) {
        connected_ = false;
        std::cout << "    [CountedSocket] 断开 " << endpoint_ << "\n";
    }
}
void CountedSocket::send(const std::string& data) {
    std::cout << "    [CountedSocket] 发送: " << data << "\n";
}

void demo_crtp_policy() {
    std::cout << "\n====== 模块4：CRTP + Policy ======\n";

    std::cout << "\n[LogPolicy — 日志策略]\n";
    {
        LoggedBuffer buf(16);
        buf[0] = 100;
        std::cout << "  buf[0] = " << buf[0] << "\n";
    }

    std::cout << "\n[SilentPolicy — 静默策略]\n";
    {
        SilentFile f("/tmp/crtp_silent.txt");
        f.write("crtp test\n");
    }

    std::cout << "\n[CountingPolicy — 统计策略]\n";
    {
        CountedSocket s1("10.0.0.1:80");
        CountedSocket s2("10.0.0.2:443");
        s1.send("GET /");
        s2.send("POST /api");
    }
    CountingPolicy::report();

    std::cout << "\n[CRTP vs 虚函数]\n";
    std::cout << "  虚函数：运行时多态，每次调用查虚表，有开销\n";
    std::cout << "  CRTP  ：编译期多态，内联展开，零运行时开销\n";
    std::cout << "  适用  ：策略固定在编译期 => CRTP；需运行时切换 => 虚函数\n";

    std::cout << "====== 模块4 结束 ======\n";
}

}  // namespace crtp
}  // namespace raii
```

---

### Task 5: scope_guard.h / scope_guard.cpp

**Files:**
- Modify: `scope_guard.h`
- Create: `scope_guard.cpp`

- [ ] **Step 1: 重写 scope_guard.h — ScopeGuard/OnFailure/OnSuccess 声明 + 模板保留**

```cpp
#pragma once
#include <functional>
#include <utility>
#include <exception>

namespace raii {
namespace scope {

class ScopeGuard {
public:
    using Action = std::function<void()>;
    explicit ScopeGuard(Action action) noexcept;
    ~ScopeGuard() noexcept;
    void dismiss() noexcept;
    ScopeGuard(const ScopeGuard&)            = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&& other) noexcept;
private:
    Action action_;
    bool   dismissed_;
};

// make_scope_guard 是函数模板，必须留在头文件
template <typename F>
[[nodiscard]] ScopeGuard make_scope_guard(F&& f) {
    return ScopeGuard(std::forward<F>(f));
}

class ScopeOnFailure {
public:
    using Action = std::function<void()>;
    explicit ScopeOnFailure(Action action) noexcept;
    ~ScopeOnFailure() noexcept;
    ScopeOnFailure(const ScopeOnFailure&)            = delete;
    ScopeOnFailure& operator=(const ScopeOnFailure&) = delete;
private:
    Action action_;
    int    exception_count_;
};

class ScopeOnSuccess {
public:
    using Action = std::function<void()>;
    explicit ScopeOnSuccess(Action action) noexcept;
    ~ScopeOnSuccess() noexcept;
    ScopeOnSuccess(const ScopeOnSuccess&)            = delete;
    ScopeOnSuccess& operator=(const ScopeOnSuccess&) = delete;
private:
    Action action_;
    int    exception_count_;
};

// SCOPE_EXIT 宏辅助（operator+ 是模板，必须在头文件）
namespace detail {
struct ScopeGuardTag {};
template <typename F>
ScopeGuard operator+(ScopeGuardTag, F&& f) {
    return make_scope_guard(std::forward<F>(f));
}
}  // namespace detail

#define SCOPE_EXIT                                                      \
    auto SCOPE_GUARD_VAR_(__scope_guard_, __LINE__) =                   \
        ::raii::scope::detail::ScopeGuardTag{} + [&]() noexcept

#define SCOPE_GUARD_VAR_(a, b) SCOPE_GUARD_CONCAT_(a, b)
#define SCOPE_GUARD_CONCAT_(a, b) a##b

void demo_scope_guard();

}  // namespace scope
}  // namespace raii
```

- [ ] **Step 2: 创建 scope_guard.cpp — ScopeGuard/OnFailure/OnSuccess 实现 + demo**

```cpp
#include "scope_guard.h"
#include <iostream>
#include <cstdio>
#include <stdexcept>

namespace raii {
namespace scope {

ScopeGuard::ScopeGuard(Action action) noexcept
    : action_(std::move(action)), dismissed_(false) {}

ScopeGuard::~ScopeGuard() noexcept {
    if (!dismissed_) {
        try { action_(); } catch (...) {}
    }
}

void ScopeGuard::dismiss() noexcept { dismissed_ = true; }

ScopeGuard::ScopeGuard(ScopeGuard&& other) noexcept
    : action_(std::move(other.action_)), dismissed_(other.dismissed_) {
    other.dismissed_ = true;
}

ScopeOnFailure::ScopeOnFailure(Action action) noexcept
    : action_(std::move(action))
    , exception_count_(std::uncaught_exceptions()) {}

ScopeOnFailure::~ScopeOnFailure() noexcept {
    if (std::uncaught_exceptions() > exception_count_) {
        try { action_(); } catch (...) {}
    }
}

ScopeOnSuccess::ScopeOnSuccess(Action action) noexcept
    : action_(std::move(action))
    , exception_count_(std::uncaught_exceptions()) {}

ScopeOnSuccess::~ScopeOnSuccess() noexcept {
    if (std::uncaught_exceptions() == exception_count_) {
        try { action_(); } catch (...) {}
    }
}

void demo_scope_guard() {
    std::cout << "\n====== 模块5：ScopeGuard ======\n";

    std::cout << "\n[基础 ScopeGuard]\n";
    {
        FILE* f = std::fopen("/tmp/scope_test.txt", "w+");
        std::cout << "  打开文件\n";
        auto guard = make_scope_guard([f]() {
            std::fclose(f);
            std::cout << "  [ScopeGuard] 自动关闭文件\n";
        });
        std::fputs("scope guard test\n", f);
        std::cout << "  写入完成，离开作用域...\n";
    }

    std::cout << "\n[dismiss — 事务提交跳过回滚]\n";
    {
        std::cout << "  开始事务\n";
        bool committed = false;
        auto rollback = make_scope_guard([&]() {
            if (!committed) std::cout << "  [ScopeGuard] 执行回滚！\n";
        });
        committed = true;
        rollback.dismiss();
        std::cout << "  事务提交成功，dismiss 跳过回滚\n";
    }

    std::cout << "\n[dismiss — 事务失败触发回滚]\n";
    {
        auto rollback = make_scope_guard([]() {
            std::cout << "  [ScopeGuard] 执行回滚（未 dismiss）\n";
        });
        std::cout << "  模拟操作失败，离开作用域时自动回滚...\n";
    }

    std::cout << "\n[ScopeOnFailure / ScopeOnSuccess]\n";
    {
        ScopeOnSuccess on_ok([]() { std::cout << "  [OnSuccess] 正常提交\n"; });
        ScopeOnFailure on_fail([]() { std::cout << "  [OnFailure] 异常回滚\n"; });
        std::cout << "  正常执行完毕...\n";
    }

    std::cout << "\n[ScopeOnFailure 异常场景]\n";
    try {
        ScopeOnSuccess on_ok([]() { std::cout << "  [OnSuccess] 不应执行\n"; });
        ScopeOnFailure on_fail([]() { std::cout << "  [OnFailure] 异常回滚触发\n"; });
        throw std::runtime_error("模拟异常");
    } catch (const std::exception& e) {
        std::cout << "  捕获异常: " << e.what() << "\n";
    }

    std::cout << "\n[SCOPE_EXIT 宏]\n";
    {
        std::cout << "  申请资源\n";
        SCOPE_EXIT {
            std::cout << "  [SCOPE_EXIT] 自动清理\n";
        };
        std::cout << "  使用资源...\n";
    }

    std::cout << "====== 模块5 结束 ======\n";
}

}  // namespace scope
}  // namespace raii
```

---

### Task 6: 更新 CMakeLists.txt

**Files:**
- Modify: `CMakeLists.txt`

- [ ] **Step 1: 添加所有 .cpp 文件到 add_executable**

```cmake
cmake_minimum_required(VERSION 3.16)
project(LearnRAII)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(LearnRAII
    main.cpp
    resource_guard.cpp
    factory_resource.cpp
    proxy_resource.cpp
    crtp_policy.cpp
    scope_guard.cpp
)

target_include_directories(LearnRAII PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
```

---

### Task 7: 构建验证

- [ ] **Step 1: 重新配置 + 构建**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Expected: `[100%] Built target LearnRAII` — 无 error，无 warning

- [ ] **Step 2: 运行验证输出一致**

```bash
./build/LearnRAII
```

Expected: 与重构前输出完全一致（所有 5 个模块正常打印构造/析构日志）
