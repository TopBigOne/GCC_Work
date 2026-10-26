/**
 * @file mmap_random_read.h
 * @brief 阶段 8：MmapRandomRead —— 用 mmap 随机读取 4 字节整数。
 */
#pragma once

#include "benchmark.h"
#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace mmap_learn {

/**
 * @brief 用 mmap 在文件中随机读取 10 万个 4 字节整数。
 *
 * 随机访问时 mmap 的优势最明显：读取任意位置只是一次指针运算 + memcpy，
 * 没有系统调用；而 read() 版本每次都要 lseek + read 两次陷入内核。
 *
 * @par 实际应用
 * - **数据库 / KV 存储**：LMDB 把整个 B+ 树文件映射进内存，查找时直接按指针跳转页面。
 * - **索引文件**：搜索引擎（Lucene 的 MMapDirectory）、地图瓦片、字典文件按偏移随机查找。
 * - **模型权重**：推理时按层随机访问权重张量，只有真正用到的页才会被加载。
 */
class MmapRandomRead : public Benchmark {
    std::string path_;                          ///< 临时测试文件路径
    FileDescriptor fd_;                         ///< 测试文件的只读 fd
    MmapRegion region_;                         ///< 整个文件的只读私有映射
    std::vector<size_t> offsets_;               ///< 预先生成的随机读取偏移（4 字节对齐）
    static constexpr size_t kNumReads = 100000; ///< 每次 execute() 的随机读取次数

protected:
    /** @brief 生成测试文件、预生成随机偏移，并映射整个文件。 */
    void setup() override;

    /**
     * @brief 按预生成的偏移从映射中读取 4 字节，累加成校验和写入 sink_。
     * @note 用 memcpy 而不是 `*(uint32_t*)(base + off)`，避免未对齐访问和严格别名问题；
     *       编译器会把 4 字节 memcpy 优化成一条 load 指令。
     */
    void execute() override;

    /** @brief 解除映射、关闭 fd、清空偏移表并删除测试文件。 */
    void teardown() override;

    /**
     * @brief 每轮实际读取的字节数：kNumReads × 4。
     * @return kNumReads * sizeof(uint32_t)
     */
    size_t bytes_processed() const override;

public:
    /**
     * @brief 构造 mmap 随机读测试。
     * @param file_size  测试文件大小（字节），必须大于 4
     * @param iterations 重复执行 kNumReads 次随机读的轮数
     */
    MmapRandomRead(size_t file_size, int iterations);
};

} // namespace mmap_learn
