#include "stage8_benchmark.h"

#include "../log.h"
#include "mmap_random_read.h"
#include "mmap_sequential_read.h"
#include "read_random_read.h"
#include "read_sequential_read.h"

#include <cstddef>
#include <string>

namespace mmap_learn {

namespace {
constexpr const char* kTag = "Stage8";
} // namespace

void Stage8Benchmark::print_header() {
    // 缩进和列宽要和 Benchmark::run() 里的数据行一致：开头 4 个空格，各列宽 30 / 13 / 18 / 12 / 17
    LOGI(kTag, "    %-30s | %13s | %18s | %12s | %17s", "Benchmark", "Throughput", "Avg Time", "Iterations",
         "Total Time");
    LOGI(kTag, "    %s-+-%s-+-%s-+-%s-+-%s", std::string(30, '-').c_str(), std::string(13, '-').c_str(),
         std::string(18, '-').c_str(), std::string(12, '-').c_str(), std::string(17, '-').c_str());
}

void Stage8Benchmark::sequential_comparison() {
    LOGD(kTag, "1-sequential_comparison: mmap vs read() sequential read");

    constexpr size_t kFileSize = 10 * 1024 * 1024; // 10 MB
    constexpr int kIterations = 5;

    print_header();

    MmapSequentialRead mmap_bench(kFileSize, kIterations);
    mmap_bench.run();

    ReadSequentialRead read_bench(kFileSize, kIterations);
    read_bench.run();

}

void Stage8Benchmark::random_comparison() {
    LOGD(kTag, "   ");
    LOGD(kTag, "   ");
    LOGD(kTag, "2-random_comparison: mmap vs lseek+read() random access");
    LOGI(kTag, "    (throughput counts bytes actually read: 100000 reads x 4 B per iteration)");

    constexpr size_t kFileSize = 10 * 1024 * 1024; // 10 MB
    constexpr int kIterations = 3;

    print_header();

    MmapRandomRead mmap_bench(kFileSize, kIterations);
    mmap_bench.run();

    ReadRandomRead read_bench(kFileSize, kIterations);
    read_bench.run();

}

void Stage8Benchmark::size_scaling() {
    LOGD(kTag, "   ");
    LOGD(kTag, "   ");
    LOGD(kTag, "3-size_scaling: Sequential read throughput at different file sizes");

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
    LOGV(kTag, "    Note: hot-cache benchmark - test files stay in page cache, no disk I/O measured");

    sequential_comparison();
    random_comparison();
    size_scaling();

    LOGI(kTag, "========================================");
    LOGI(kTag, "Stage 8 complete");
    LOGI(kTag, "========================================");
}

} // namespace mmap_learn
