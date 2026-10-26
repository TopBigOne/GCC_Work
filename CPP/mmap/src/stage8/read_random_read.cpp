#include "read_random_read.h"

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <random>
#include <unistd.h>

namespace mmap_learn {

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

} // namespace mmap_learn
