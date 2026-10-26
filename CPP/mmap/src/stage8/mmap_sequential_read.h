/**
 * @file mmap_sequential_read.h
 * @brief 阶段 8：MmapSequentialRead —— 用 mmap 顺序读取整个文件。
 */
#pragma once

#include "benchmark.h"
#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <string>

namespace mmap_learn {

/**
 * @brief 用 mmap 顺序读取整个文件，逐字节求和。
 *
 * 映射后直接通过指针遍历内存，没有 read() 调用、没有用户态缓冲区拷贝；
 * 第一次 execute() 会为每一页触发缺页中断，之后的迭代页表已建立，基本是纯内存访问。
 *
 * @par 实际应用
 * 适合"一次映射、反复扫描"的大文件，例如日志分析工具扫描 GB 级日志、
 * 编译器/链接器读取目标文件、ripgrep 搜索单个大文件。
 * 对只读一遍的小文件，mmap + munmap + 缺页的固定开销可能比 read() 更大。
 */
class MmapSequentialRead : public Benchmark {
    std::string path_;   ///< 临时测试文件路径
    FileDescriptor fd_;  ///< 测试文件的只读 fd
    MmapRegion region_;  ///< 整个文件的只读私有映射

protected:
    /** @brief 生成随机内容的测试文件，并以只读 + MAP_PRIVATE 映射整个文件。 */
    void setup() override;

    /**
     * @brief 通过映射指针顺序读取每个字节，累加成校验和写入 sink_。
     * @note 循环里用普通指针和局部变量累加，编译器可以向量化；
     *       只在最后把结果写入 volatile 的 sink_，防止整个循环被当作"无用计算"删除。
     *       read() 版本用完全相同的累加方式，两边对比才公平。
     */
    void execute() override;

    /** @brief 通过移动赋值空对象触发 munmap 与 close，再删除测试文件。 */
    void teardown() override;

public:
    /**
     * @brief 构造 mmap 顺序读测试。
     * @param file_size  测试文件大小（字节）
     * @param iterations 顺序扫描的次数
     */
    MmapSequentialRead(size_t file_size, int iterations);
};

} // namespace mmap_learn
