/**
 * @file read_sequential_read.h
 * @brief 阶段 8：ReadSequentialRead —— 用 read() 顺序读取整个文件（对照组）。
 */
#pragma once

#include "benchmark.h"
#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <string>

namespace mmap_learn {

/**
 * @brief 用 read() 以 64KB 缓冲区顺序读取整个文件，逐字节求和，作为 mmap 的对照组。
 *
 * 每次 read() 都是一次系统调用，并把数据从 page cache 拷贝到用户缓冲区；
 * 但缓冲区固定 64KB、常驻 CPU 缓存，且没有缺页和页表开销。
 *
 * @par 实际应用
 * 流式处理（cp、tar、gzip、网络上传）只顺序读一遍，通常用 read() 配合合适的缓冲区大小，
 * 可预测性好，I/O 错误以返回值形式出现，而不是 mmap 的 SIGBUS 信号。
 */
class ReadSequentialRead : public Benchmark {
    std::string path_;   ///< 临时测试文件路径
    FileDescriptor fd_;  ///< 测试文件的只读 fd

protected:
    /** @brief 生成与 MmapSequentialRead 相同内容的测试文件并以只读方式打开。 */
    void setup() override;

    /** @brief 回到文件开头，循环 read() 到 64KB 缓冲区，按与 mmap 版本相同的方式累加校验和。 */
    void execute() override;

    /** @brief 关闭 fd 并删除测试文件。 */
    void teardown() override;

public:
    /**
     * @brief 构造 read() 顺序读测试。
     * @param file_size  测试文件大小（字节）
     * @param iterations 顺序扫描的次数
     */
    ReadSequentialRead(size_t file_size, int iterations);
};

} // namespace mmap_learn
