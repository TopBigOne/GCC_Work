/**
 * @file benchmark.h
 * @brief 阶段 8：Benchmark —— 基准测试抽象基类（模板方法）。
 */
#pragma once

#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace mmap_learn {

/**
 * @brief 基准测试抽象基类，使用模板方法（Template Method）模式。
 *
 * 基类在 run() 中固定测试流程：setup → 计时循环 execute → teardown → 打印报告；
 * 子类只需实现三个纯虚函数，描述"准备什么、测什么、怎么清理"。
 * 这样所有 benchmark 的计时方式、统计口径完全一致，对比结果才公平。
 *
 * @par 实际应用
 * Google Benchmark、JMH 等框架都是同样的思路：把计时、迭代、统计放在框架里，
 * 用户只写被测代码。做 mmap 与 read 的技术选型时，应该用这类统一框架在
 * 目标机器、真实数据规模上实测，而不是凭经验判断。
 */
class Benchmark {
protected:
    std::string name_;  ///< 测试名称，用于报告输出
    size_t file_size_;  ///< 测试文件大小（字节），也用于计算吞吐量
    int iterations_;    ///< execute() 的重复次数

    /**
     * 计算结果的"汇点"：execute() 把校验和写到这里，
     * 编译器无法证明结果没人用，就不能把整个读取循环优化掉。
     * 只在每轮结束写一次，不影响循环本身的向量化。
     */
    volatile uint64_t sink_ = 0;

    /** @brief 准备测试环境（创建文件、建立映射等），不计入耗时。 */
    virtual void setup() = 0;

    /** @brief 被测的核心操作，执行 iterations_ 次并计时。 */
    virtual void execute() = 0;

    /** @brief 清理测试环境（解除映射、关闭 fd、删除文件），不计入耗时。 */
    virtual void teardown() = 0;

    /**
     * @brief 创建内容为固定随机字节的测试文件，供各子类的 setup() 使用。
     *
     * 用传统 I/O 每 64KB 一块写入随机数据，随机种子固定为 42，
     * 保证所有 benchmark 读到的文件内容完全一致。
     *
     * @param path 测试文件路径，已存在时会被清空重写
     * @throw std::runtime_error 打开文件或 ftruncate 失败时抛出
     */
    void create_test_file(const std::string &path) const;

    /**
     * @brief 每次 execute() 实际处理的字节数，用于计算吞吐量。
     *
     * 默认是整个文件大小（顺序读会读完整个文件）；
     * 随机读子类只读 kNumReads × 4 字节，需要覆盖此函数，否则 MB/s 会被严重夸大。
     *
     * @return 单次 execute() 读取的字节数
     */
    virtual size_t bytes_processed() const;

public:
    /**
     * @brief 构造基准测试。
     * @param name       测试名称
     * @param file_size  测试文件大小（字节）
     * @param iterations execute() 的重复次数
     */
    Benchmark(std::string name, size_t file_size, int iterations);

    /** @brief 虚析构，保证通过基类指针销毁子类时行为正确。 */
    virtual ~Benchmark() = default;

    /**
     * @brief 模板方法：执行完整测试流程并打印吞吐量、平均耗时、总耗时。
     *
     * 吞吐量 = bytes_processed() × iterations_ / 总耗时（MB/s），
     * 即按"实际读取的字节数"计算，顺序读与随机读的数字含义一致。
     */
    void run();
};

} // namespace mmap_learn
