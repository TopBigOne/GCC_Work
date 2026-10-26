#include "benchmark.h"

#include "../log.h"

#include <algorithm>
#include <chrono>
#include <fcntl.h>
#include <random>
#include <unistd.h>
#include <utility>
#include <vector>

namespace mmap_learn {

namespace {
constexpr const char* kTag = "Stage8";
} // namespace

Benchmark::Benchmark(std::string name, size_t file_size, int iterations)
    : name_(std::move(name)), file_size_(file_size), iterations_(iterations) {}

void Benchmark::create_test_file(const std::string &path) const {
    // 用传统 I/O 创建测试文件，每 64KB 一块写入随机数据（固定种子 42，保证每次内容一致）
    FileDescriptor wfd(path.c_str(), O_CREAT | O_RDWR | O_TRUNC);
    wfd.truncate(static_cast<off_t>(file_size_));

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 255);
    constexpr size_t kBufSize = 65536;
    std::vector<char> buf(kBufSize);
    size_t remaining = file_size_;
    off_t offset = 0;

    while (remaining > 0) {
        size_t chunk = std::min(remaining, kBufSize);
        for (size_t i = 0; i < chunk; ++i) {
            buf[i] = static_cast<char>(dist(rng));
        }
        ::pwrite(wfd.get(), buf.data(), chunk, offset);
        offset += static_cast<off_t>(chunk);
        remaining -= chunk;
    }
}

size_t Benchmark::bytes_processed() const {
    return file_size_;
}

void Benchmark::run() {
    setup();

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations_; ++i) {
        execute();
    }
    auto end = std::chrono::high_resolution_clock::now();

    teardown();

    double elapsed_sec = std::chrono::duration<double>(end - start).count();
    double avg_ms = (elapsed_sec * 1000.0) / iterations_;
    // 按每轮实际读取的字节数计算吞吐量（随机读只读了一小部分文件）
    double total_bytes = static_cast<double>(bytes_processed()) * iterations_;
    double throughput_mbs = (total_bytes / (1024.0 * 1024.0)) / elapsed_sec;

    // 列宽要和 print_header() 一致：13（%8.2f MB/s）、18（%10.3f ms/iter）、12（%6d iters）、17（%8.2f ms total）
    LOGI(kTag, "    %-30s | %8.2f MB/s | %10.3f ms/iter | %6d iters | %8.2f ms total", name_.c_str(), throughput_mbs,
         avg_ms, iterations_, elapsed_sec * 1000.0);
}

} // namespace mmap_learn
