#include "mmap_sequential_read.h"

#include <cstdint>
#include <fcntl.h>
#include <unistd.h>

namespace mmap_learn {

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

} // namespace mmap_learn
