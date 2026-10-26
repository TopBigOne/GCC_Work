#include "mmap_random_read.h"

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <random>
#include <unistd.h>

namespace mmap_learn {

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

} // namespace mmap_learn
