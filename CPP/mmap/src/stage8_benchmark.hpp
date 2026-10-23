#pragma once

#include "mmap_wrapper.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <random>
#include <string>
#include <unistd.h>
#include <vector>

namespace mmap_learn {

// ============================================================
// Abstract Base: Benchmark (Template Method Pattern)
// ============================================================
class Benchmark {
protected:
    std::string name_;
    size_t file_size_;
    int iterations_;

    virtual void setup() = 0;
    virtual void execute() = 0;
    virtual void teardown() = 0;

public:
    Benchmark(std::string name, size_t file_size, int iterations)
        : name_(std::move(name)), file_size_(file_size), iterations_(iterations) {}

    virtual ~Benchmark() = default;

    // Template Method: setup -> timed execute loop -> teardown -> report
    void run() {
        setup();

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations_; ++i) {
            execute();
        }
        auto end = std::chrono::high_resolution_clock::now();

        teardown();

        double elapsed_sec = std::chrono::duration<double>(end - start).count();
        double avg_ms = (elapsed_sec * 1000.0) / iterations_;
        double total_bytes = static_cast<double>(file_size_) * iterations_;
        double throughput_mbs = (total_bytes / (1024.0 * 1024.0)) / elapsed_sec;

        printf("  %-30s | %8.2f MB/s | %10.3f ms/iter | %6d iters | %8.2f ms total\n",
               name_.c_str(), throughput_mbs, avg_ms, iterations_, elapsed_sec * 1000.0);
    }
};

// ============================================================
// MmapSequentialRead: Sequential read via mmap
// ============================================================
class MmapSequentialRead : public Benchmark {
    std::string path_;
    FileDescriptor fd_;
    MmapRegion region_;

protected:
    void setup() override {
        path_ = "/tmp/mmap_bench_seq_mmap.dat";

        // Create temp file filled with random data
        {
            FileDescriptor wfd(path_.c_str(), O_CREAT | O_RDWR | O_TRUNC);
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

        // Open and mmap for reading
        fd_ = FileDescriptor(path_.c_str(), O_RDONLY);
        region_ = MmapBuilder()
                      .length(file_size_)
                      .readOnly()
                      .priv()
                      .file(fd_.get())
                      .build();
    }

    void execute() override {
        const volatile unsigned char *ptr = static_cast<const unsigned char *>(region_.data());
        volatile unsigned long sum = 0;
        for (size_t i = 0; i < file_size_; ++i) {
            sum += ptr[i];
        }
        (void)sum;
    }

    void teardown() override {
        region_ = MmapRegion();
        fd_ = FileDescriptor();
        ::unlink(path_.c_str());
    }

public:
    MmapSequentialRead(size_t file_size, int iterations)
        : Benchmark("mmap sequential read", file_size, iterations) {}
};

// ============================================================
// ReadSequentialRead: Sequential read via read() syscall
// ============================================================
class ReadSequentialRead : public Benchmark {
    std::string path_;
    FileDescriptor fd_;

protected:
    void setup() override {
        path_ = "/tmp/mmap_bench_seq_read.dat";

        // Create temp file filled with random data
        {
            FileDescriptor wfd(path_.c_str(), O_CREAT | O_RDWR | O_TRUNC);
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

        fd_ = FileDescriptor(path_.c_str(), O_RDONLY);
    }

    void execute() override {
        ::lseek(fd_.get(), 0, SEEK_SET);

        constexpr size_t kBufSize = 65536;
        char buf[kBufSize];
        volatile unsigned long sum = 0;
        size_t remaining = file_size_;

        while (remaining > 0) {
            size_t to_read = std::min(remaining, kBufSize);
            ssize_t n = ::read(fd_.get(), buf, to_read);
            if (n <= 0) break;
            for (ssize_t i = 0; i < n; ++i) {
                sum += static_cast<unsigned char>(buf[i]);
            }
            remaining -= static_cast<size_t>(n);
        }
        (void)sum;
    }

    void teardown() override {
        fd_ = FileDescriptor();
        ::unlink(path_.c_str());
    }

public:
    ReadSequentialRead(size_t file_size, int iterations)
        : Benchmark("read() sequential read", file_size, iterations) {}
};

// ============================================================
// MmapRandomRead: Random 4-byte reads via mmap
// ============================================================
class MmapRandomRead : public Benchmark {
    std::string path_;
    FileDescriptor fd_;
    MmapRegion region_;
    std::vector<size_t> offsets_;
    static constexpr size_t kNumReads = 100000;

protected:
    void setup() override {
        path_ = "/tmp/mmap_bench_rand_mmap.dat";

        // Create temp file filled with random data
        {
            FileDescriptor wfd(path_.c_str(), O_CREAT | O_RDWR | O_TRUNC);
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

        // Pre-generate random offsets (aligned to 4-byte boundary, within bounds)
        size_t max_offset = file_size_ - 4;
        std::mt19937 rng(123);
        std::uniform_int_distribution<size_t> dist(0, max_offset / 4);
        offsets_.resize(kNumReads);
        for (size_t i = 0; i < kNumReads; ++i) {
            offsets_[i] = dist(rng) * 4;
        }

        // Open and mmap
        fd_ = FileDescriptor(path_.c_str(), O_RDONLY);
        region_ = MmapBuilder()
                      .length(file_size_)
                      .readOnly()
                      .priv()
                      .file(fd_.get())
                      .build();
    }

    void execute() override {
        const unsigned char *base = static_cast<const unsigned char *>(region_.data());
        volatile uint32_t val = 0;
        for (size_t i = 0; i < kNumReads; ++i) {
            uint32_t v;
            std::memcpy(&v, base + offsets_[i], sizeof(v));
            val = v;
        }
        (void)val;
    }

    void teardown() override {
        region_ = MmapRegion();
        fd_ = FileDescriptor();
        offsets_.clear();
        ::unlink(path_.c_str());
    }

public:
    MmapRandomRead(size_t file_size, int iterations)
        : Benchmark("mmap random read", file_size, iterations) {}
};

// ============================================================
// ReadRandomRead: Random 4-byte reads via lseek+read
// ============================================================
class ReadRandomRead : public Benchmark {
    std::string path_;
    FileDescriptor fd_;
    std::vector<size_t> offsets_;
    static constexpr size_t kNumReads = 100000;

protected:
    void setup() override {
        path_ = "/tmp/mmap_bench_rand_read.dat";

        // Create temp file filled with random data
        {
            FileDescriptor wfd(path_.c_str(), O_CREAT | O_RDWR | O_TRUNC);
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

        // Pre-generate random offsets (same seed as MmapRandomRead for fair comparison)
        size_t max_offset = file_size_ - 4;
        std::mt19937 rng(123);
        std::uniform_int_distribution<size_t> dist(0, max_offset / 4);
        offsets_.resize(kNumReads);
        for (size_t i = 0; i < kNumReads; ++i) {
            offsets_[i] = dist(rng) * 4;
        }

        fd_ = FileDescriptor(path_.c_str(), O_RDONLY);
    }

    void execute() override {
        volatile uint32_t val = 0;
        char buf[4];
        for (size_t i = 0; i < kNumReads; ++i) {
            ::lseek(fd_.get(), static_cast<off_t>(offsets_[i]), SEEK_SET);
            ssize_t n = ::read(fd_.get(), buf, 4);
            if (n == 4) {
                uint32_t v;
                std::memcpy(&v, buf, sizeof(v));
                val = v;
            }
        }
        (void)val;
    }

    void teardown() override {
        fd_ = FileDescriptor();
        offsets_.clear();
        ::unlink(path_.c_str());
    }

public:
    ReadRandomRead(size_t file_size, int iterations)
        : Benchmark("read() random read", file_size, iterations) {}
};

// ============================================================
// Stage8Benchmark: Orchestrator with demo methods
// ============================================================
class Stage8Benchmark {
    static void print_header() {
        printf("  %-30s | %11s | %14s | %12s | %14s\n",
               "Benchmark", "Throughput", "Avg Time", "Iterations", "Total Time");
        printf("  %-30s-+-%11s-+-%14s-+-%12s-+-%14s\n",
               "------------------------------", "-----------", "--------------", "------------", "--------------");
    }

public:
    // ============================================================
    // Demo 1: Sequential read comparison (mmap vs read)
    // ============================================================
    static void demo_sequential_comparison() {
        printf("[Stage8] demo_sequential_comparison: mmap vs read() sequential read\n\n");

        constexpr size_t kFileSize = 10 * 1024 * 1024; // 10 MB
        constexpr int kIterations = 5;

        print_header();

        MmapSequentialRead mmap_bench(kFileSize, kIterations);
        mmap_bench.run();

        ReadSequentialRead read_bench(kFileSize, kIterations);
        read_bench.run();

        printf("\n");
    }

    // ============================================================
    // Demo 2: Random access comparison (mmap vs lseek+read)
    // ============================================================
    static void demo_random_comparison() {
        printf("[Stage8] demo_random_comparison: mmap vs lseek+read() random access\n\n");

        constexpr size_t kFileSize = 10 * 1024 * 1024; // 10 MB
        constexpr int kIterations = 3;

        print_header();

        MmapRandomRead mmap_bench(kFileSize, kIterations);
        mmap_bench.run();

        ReadRandomRead read_bench(kFileSize, kIterations);
        read_bench.run();

        printf("\n");
    }

    // ============================================================
    // Demo 3: Scaling behavior across different file sizes
    // ============================================================
    static void demo_size_scaling() {
        printf("[Stage8] demo_size_scaling: Sequential read throughput at different file sizes\n\n");

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
            printf("  --- File size: %s ---\n", cfg.label);
            print_header();

            MmapSequentialRead mmap_bench(cfg.size, kIterations);
            mmap_bench.run();

            ReadSequentialRead read_bench(cfg.size, kIterations);
            read_bench.run();

            printf("\n");
        }
    }

    // ============================================================
    // Run all Stage 8 demos
    // ============================================================
    static void run_all() {
        printf("========================================\n");
        printf("  Stage 8: Performance Benchmarking\n");
        printf("========================================\n\n");

        demo_sequential_comparison();
        demo_random_comparison();
        demo_size_scaling();

        printf("========================================\n");
        printf("  Stage 8 complete\n");
        printf("========================================\n");
    }
};

} // namespace mmap_learn
