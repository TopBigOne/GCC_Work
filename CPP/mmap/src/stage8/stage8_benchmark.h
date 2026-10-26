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

namespace mmap_learn {

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
    static void sequential_comparison();

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
    static void random_comparison();

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
    static void size_scaling();

    /** @brief 依次运行阶段 8 的全部 demo，开头提示本阶段是热缓存测试。 */
    static void run_all();
};

} // namespace mmap_learn
