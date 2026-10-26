#include "stage8_benchmark.h"

#include "../log.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <random>
#include <unistd.h>
#include <utility>

namespace mmap_learn {

namespace {
constexpr const char* kTag = "Stage8";
} // namespace

// ============================================================
// Benchmark
// ============================================================

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

// ============================================================
// MmapSequentialRead
// ============================================================

MmapSequentialRead::MmapSequentialRead(size_t file_size, int iterations)
    : Benchmark("mmap sequential read", file_size, iterations) {}

void MmapSequentialRead::setup() {
    path_ = work_path("mmap_bench_seq_mmap.dat");
    create_test_file(path_);

    // 以只读方式打开并映射整个文件
    fd_ = FileDescriptor(path_.c_str(), O_RDONLY);
    region_ = MmapBuilder()
                  .length(file_size_)
                  .readOnly()
                  .priv()
                  .file(fd_.get())
                  .build();
}

void MmapSequentialRead::execute() {
    // 普通指针 + 局部变量累加，允许编译器向量化；最后写一次 sink_ 防止循环被优化掉
    const unsigned char *ptr = static_cast<const unsigned char *>(region_.data());
    uint64_t sum = 0;
    for (size_t i = 0; i < file_size_; ++i) {
        sum += ptr[i];
    }
    sink_ = sum;
}

void MmapSequentialRead::teardown() {
    region_ = MmapRegion();
    fd_ = FileDescriptor();
    ::unlink(path_.c_str());
}

// ============================================================
// ReadSequentialRead
// ============================================================

ReadSequentialRead::ReadSequentialRead(size_t file_size, int iterations)
    : Benchmark("read() sequential read", file_size, iterations) {}

void ReadSequentialRead::setup() {
    path_ = work_path("mmap_bench_seq_read.dat");
    create_test_file(path_);

    fd_ = FileDescriptor(path_.c_str(), O_RDONLY);
}

void ReadSequentialRead::execute() {
    // 每次迭代都从头读，保证与 mmap 版本读取同样的数据量
    ::lseek(fd_.get(), 0, SEEK_SET);

    constexpr size_t kBufSize = 65536;
    unsigned char buf[kBufSize];
    uint64_t sum = 0;
    size_t remaining = file_size_;

    while (remaining > 0) {
        size_t to_read = std::min(remaining, kBufSize);
        ssize_t n = ::read(fd_.get(), buf, to_read);
        if (n <= 0) break;
        // 与 mmap 版本完全相同的累加方式，保证对比公平
        for (ssize_t i = 0; i < n; ++i) {
            sum += buf[i];
        }
        remaining -= static_cast<size_t>(n);
    }
    sink_ = sum;
}

void ReadSequentialRead::teardown() {
    fd_ = FileDescriptor();
    ::unlink(path_.c_str());
}

// ============================================================
// MmapRandomRead
// ============================================================

MmapRandomRead::MmapRandomRead(size_t file_size, int iterations)
    : Benchmark("mmap random read", file_size, iterations) {}

void MmapRandomRead::setup() {
    path_ = work_path("mmap_bench_rand_mmap.dat");
    create_test_file(path_);

    // 预先生成随机偏移（4 字节对齐且不越界），避免把随机数生成的耗时算进测试
    size_t max_offset = file_size_ - 4;
    std::mt19937 rng(123);
    std::uniform_int_distribution<size_t> dist(0, max_offset / 4);
    offsets_.resize(kNumReads);
    for (size_t i = 0; i < kNumReads; ++i) {
        offsets_[i] = dist(rng) * 4;
    }

    // 以只读方式打开并映射整个文件
    fd_ = FileDescriptor(path_.c_str(), O_RDONLY);
    region_ = MmapBuilder()
                  .length(file_size_)
                  .readOnly()
                  .priv()
                  .file(fd_.get())
                  .build();
}

void MmapRandomRead::execute() {
    const unsigned char *base = static_cast<const unsigned char *>(region_.data());
    uint64_t sum = 0;
    for (size_t i = 0; i < kNumReads; ++i) {
        uint32_t v;
        std::memcpy(&v, base + offsets_[i], sizeof(v));
        sum += v;
    }
    sink_ = sum;
}

size_t MmapRandomRead::bytes_processed() const {
    return kNumReads * sizeof(uint32_t);
}

void MmapRandomRead::teardown() {
    region_ = MmapRegion();
    fd_ = FileDescriptor();
    offsets_.clear();
    ::unlink(path_.c_str());
}

// ============================================================
// ReadRandomRead
// ============================================================

ReadRandomRead::ReadRandomRead(size_t file_size, int iterations)
    : Benchmark("read() random read", file_size, iterations) {}

void ReadRandomRead::setup() {
    path_ = work_path("mmap_bench_rand_read.dat");
    create_test_file(path_);

    // 预先生成随机偏移，种子与 MmapRandomRead 相同（123），保证两边访问完全相同的位置
    size_t max_offset = file_size_ - 4;
    std::mt19937 rng(123);
    std::uniform_int_distribution<size_t> dist(0, max_offset / 4);
    offsets_.resize(kNumReads);
    for (size_t i = 0; i < kNumReads; ++i) {
        offsets_[i] = dist(rng) * 4;
    }

    fd_ = FileDescriptor(path_.c_str(), O_RDONLY);
}

void ReadRandomRead::execute() {
    uint64_t sum = 0;
    char buf[4];
    for (size_t i = 0; i < kNumReads; ++i) {
        ::lseek(fd_.get(), static_cast<off_t>(offsets_[i]), SEEK_SET);
        ssize_t n = ::read(fd_.get(), buf, 4);
        if (n == 4) {
            uint32_t v;
            std::memcpy(&v, buf, sizeof(v));
            sum += v;
        }
    }
    sink_ = sum;
}

size_t ReadRandomRead::bytes_processed() const {
    return kNumReads * sizeof(uint32_t);
}

void ReadRandomRead::teardown() {
    fd_ = FileDescriptor();
    offsets_.clear();
    ::unlink(path_.c_str());
}

// ============================================================
// Stage8Benchmark
// ============================================================

void Stage8Benchmark::print_header() {
    // 缩进和列宽要和 Benchmark::run() 里的数据行一致：开头 4 个空格，各列宽 30 / 13 / 18 / 12 / 17
    LOGI(kTag, "    %-30s | %13s | %18s | %12s | %17s", "Benchmark", "Throughput", "Avg Time", "Iterations",
         "Total Time");
    LOGI(kTag, "    %s-+-%s-+-%s-+-%s-+-%s", std::string(30, '-').c_str(), std::string(13, '-').c_str(),
         std::string(18, '-').c_str(), std::string(12, '-').c_str(), std::string(17, '-').c_str());
}

void Stage8Benchmark::demo_sequential_comparison() {
    LOGD(kTag, "1-demo_sequential_comparison: mmap vs read() sequential read");

    constexpr size_t kFileSize = 10 * 1024 * 1024; // 10 MB
    constexpr int kIterations = 5;

    print_header();

    MmapSequentialRead mmap_bench(kFileSize, kIterations);
    mmap_bench.run();

    ReadSequentialRead read_bench(kFileSize, kIterations);
    read_bench.run();

}

void Stage8Benchmark::demo_random_comparison() {
    LOGD(kTag, "   ");
    LOGD(kTag, "   ");
    LOGD(kTag, "2-demo_random_comparison: mmap vs lseek+read() random access");
    LOGI(kTag, "    (throughput counts bytes actually read: 100000 reads x 4 B per iteration)");

    constexpr size_t kFileSize = 10 * 1024 * 1024; // 10 MB
    constexpr int kIterations = 3;

    print_header();

    MmapRandomRead mmap_bench(kFileSize, kIterations);
    mmap_bench.run();

    ReadRandomRead read_bench(kFileSize, kIterations);
    read_bench.run();

}

void Stage8Benchmark::demo_size_scaling() {
    LOGD(kTag, "   ");
    LOGD(kTag, "   ");
    LOGD(kTag, "3-demo_size_scaling: Sequential read throughput at different file sizes");

    struct SizeConfig {
        size_t size;
        const char *label;
    };

    SizeConfig sizes[] = {
        {1  * 1024 * 1024, "1 MB"},
        {10 * 1024 * 1024, "10 MB"},
        {50 * 1024 * 1024, "50 MB"},
    };

    constexpr int kIterations = 3;

    for (const auto &cfg : sizes) {
        LOGI(kTag, "    --- File size: %s ---", cfg.label);
        print_header();

        MmapSequentialRead mmap_bench(cfg.size, kIterations);
        mmap_bench.run();

        ReadSequentialRead read_bench(cfg.size, kIterations);
        read_bench.run();

    }
}

void Stage8Benchmark::run_all() {
    LOGI(kTag, "========================================");
    LOGI(kTag, "Stage 8: Performance Benchmarking");
    LOGI(kTag, "========================================");
    LOGD(kTag, "Note: hot-cache benchmark - test files stay in page cache, no disk I/O measured");

    demo_sequential_comparison();
    demo_random_comparison();
    demo_size_scaling();

    LOGI(kTag, "========================================");
    LOGI(kTag, "Stage 8 complete");
    LOGI(kTag, "========================================");
}

} // namespace mmap_learn
