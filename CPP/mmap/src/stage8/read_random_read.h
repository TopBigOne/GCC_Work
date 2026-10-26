/**
 * @file read_random_read.h
 * @brief 阶段 8：ReadRandomRead —— 用 lseek + read() 随机读取 4 字节整数（对照组）。
 */
#pragma once

#include "benchmark.h"
#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace mmap_learn {

/**
 * @brief 用 lseek + read() 在文件中随机读取 10 万个 4 字节整数，作为 mmap 的对照组。
 *
 * 每次读取 2 次系统调用，读 4 字节却要付出完整的用户态/内核态切换开销，
 * 通常明显慢于 mmap 版本。
 *
 * @par 实际应用
 * 不想用 mmap 时，随机读应改用 pread()（一次系统调用、线程安全，无需共享文件偏移），
 * 或像 InnoDB / RocksDB 一样按页读入并在用户态缓存，把小读取合并成大读取。
 */
class ReadRandomRead : public Benchmark {
    std::string path_;                          ///< 临时测试文件路径
    FileDescriptor fd_;                         ///< 测试文件的只读 fd
    std::vector<size_t> offsets_;               ///< 预先生成的随机读取偏移（4 字节对齐）
    static constexpr size_t kNumReads = 100000; ///< 每次 execute() 的随机读取次数

protected:
    /** @brief 生成测试文件、预生成与 mmap 版本相同的随机偏移，并打开文件。 */
    void setup() override;

    /** @brief 对每个偏移执行 lseek 定位 + read 4 字节，累加成校验和写入 sink_。 */
    void execute() override;

    /** @brief 关闭 fd、清空偏移表并删除测试文件。 */
    void teardown() override;

    /**
     * @brief 每轮实际读取的字节数：kNumReads × 4。
     * @return kNumReads * sizeof(uint32_t)
     */
    size_t bytes_processed() const override;

public:
    /**
     * @brief 构造 read() 随机读测试。
     * @param file_size  测试文件大小（字节），必须大于 4
     * @param iterations 重复执行 kNumReads 次随机读的轮数
     */
    ReadRandomRead(size_t file_size, int iterations);
};

} // namespace mmap_learn
