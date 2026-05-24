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

FILE* FileGuard::get() const noexcept  { return file_; }
bool  FileGuard::is_open() const noexcept { return file_ != nullptr; }

void FileGuard::write(const std::string& text) {
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
