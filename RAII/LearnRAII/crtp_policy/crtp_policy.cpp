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

// ---- demo ----

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
