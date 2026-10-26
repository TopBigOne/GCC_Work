/**
 * @file stage8_benchmark.h
 * @brief 阶段 8：性能与取舍 —— 用基准测试对比 mmap 与 read() 的读取性能。
 *
 * 本阶段学什么：
 * - 顺序读：mmap 直接访问 page cache vs read() 把数据拷贝到用户缓冲区
 * - 随机读：mmap 指针运算 vs 每次 lseek + read 两次系统调用
 * - 文件大小对吞吐量的影响（缺页次数、TLB 压力随映射增大而变化）
 *
 * 对应的 mmap 知识点：
 * - mmap 省掉一次内核态 → 用户态的数据拷贝，也省掉每次读取的系统调用开销
 * - 但首次访问每一页都会触发缺页中断（page fault），页越多开销越大
 * - read() 的缓冲区可以复用、对 CPU 缓存友好；内核对顺序 read 也有预读
 *
 * @par 实际应用
 * - **ripgrep / grep**：ripgrep 对单个大文件用 mmap、对大量小文件用 read，
 *   因为小文件 mmap/munmap 的建立与拆除开销反而超过拷贝开销。
 * - **数据库**：LMDB、SQLite（mmap_size 选项）、MongoDB 旧版 MMAPv1 用 mmap 做随机读；
 *   而 PostgreSQL、MySQL InnoDB 坚持用 read/pread + 自管缓冲池，以便控制刷盘与淘汰。
 * - **模型 / 资源加载**：llama.cpp 用 mmap 加载 GB 级模型权重，启动快、多进程共享物理页。
 * 结论：没有绝对更快的一方，要按访问模式、文件大小、是否多进程共享来选择，并用 benchmark 验证。
 *
 * @note 测试文件写完后仍在 page cache 中，测的是"热缓存"性能，不含磁盘 I/O；
 *       Linux 上可用 `echo 3 > /proc/sys/vm/drop_caches` 测冷缓存，macOS 上用 `sudo purge`。
 */
#pragma once

#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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

/**
 * @brief 阶段 8 入口：组织各组对照 benchmark 并打印结果表格。
 */
class Stage8Benchmark {
    /** @brief 打印结果表格的表头。 */
    static void print_header();

public:
    /**
     * @brief Demo 1：10MB 文件顺序读 5 次，对比 mmap 与 read()。
     *
     * 热缓存下两者通常差距不大：mmap 省了拷贝，但 read() 省了缺页与页表开销。
     *
     * @par 实际应用
     * 判断"顺序扫描大文件"用哪种方式。对只扫一遍的流式场景，read() 往往足够；
     * 需要反复扫描或多进程共享同一文件时，mmap 更有优势。
     */
    static void demo_sequential_comparison();

    /**
     * @brief Demo 2：10MB 文件内随机读取 10 万个 4 字节，对比 mmap 与 lseek + read()。
     *
     * 预期 mmap 明显更快，因为它把 20 万次系统调用变成了普通内存访问。
     * 吞吐量按实际读取量（每轮 10 万 × 4 字节）计算，所以数值远小于顺序读。
     *
     * @par 实际应用
     * 这正是 LMDB、Lucene、各类索引文件选择 mmap 的原因：小粒度随机访问时，
     * 系统调用开销远大于数据本身的读取开销。
     */
    static void demo_random_comparison();

    /**
     * @brief Demo 3：在 1MB / 10MB / 50MB 三种文件大小下对比顺序读吞吐量。
     *
     * 观察吞吐量随文件大小的变化：文件越大，缺页次数和 TLB 未命中越多，
     * 而 read() 的 64KB 缓冲区始终留在 CPU 缓存中。
     *
     * @par 实际应用
     * 选型时要用接近生产的数据规模测试：小数据上得出的结论在大数据上可能反转。
     * 对超大映射可用 madvise(MADV_SEQUENTIAL) 加强预读，或用大页（Linux 的 THP /
     * MAP_HUGETLB）降低 TLB 压力。
     * @note MAP_HUGETLB 只在 Linux 上可用，macOS 没有对应的用户态大页接口。
     */
    static void demo_size_scaling();

    /** @brief 依次运行阶段 8 的全部 demo，开头提示本阶段是热缓存测试。 */
    static void run_all();
};

} // namespace mmap_learn
