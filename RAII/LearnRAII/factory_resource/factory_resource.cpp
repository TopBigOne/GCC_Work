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

void TempFileResource::use() {
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
