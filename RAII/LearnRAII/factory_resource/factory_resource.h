#pragma once
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

// ============================================================
// 模块2：工厂模式 + RAII
// 知识点：
//   1. 抽象接口 IResource 统一资源行为
//   2. 工厂方法返回 unique_ptr，所有权清晰
//   3. 多态析构（虚析构函数）
// ============================================================

namespace raii {
namespace factory {

// ----------------------------------------------------------
// 2.1 抽象资源接口
// ----------------------------------------------------------
class IResource {
public:
    virtual ~IResource() = default;
    virtual void        use()                     = 0;
    virtual std::string name() const              = 0;
    virtual bool        is_valid() const noexcept = 0;
};

// ----------------------------------------------------------
// 2.2 具体资源声明
// ----------------------------------------------------------
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

// ----------------------------------------------------------
// 2.3 资源工厂（纯静态）
// ----------------------------------------------------------
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
