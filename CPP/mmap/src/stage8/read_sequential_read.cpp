#include "read_sequential_read.h"

#include <algorithm>
#include <cstdint>
#include <fcntl.h>
#include <unistd.h>

namespace mmap_learn {

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

} // namespace mmap_learn
