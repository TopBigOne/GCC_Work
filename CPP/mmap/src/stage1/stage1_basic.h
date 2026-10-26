/**
 * @file stage1_basic.h
 * @brief 阶段 1：mmap 基本用法 —— 用 mmap 读文件、写文件，对比 MAP_SHARED 与 MAP_PRIVATE。
 *
 * 本阶段对应的 mmap 知识点：
 * - mmap(2) 的 6 个参数：addr / length / prot / flags / fd / offset
 * ----------------------------------------------------------------------
 * - prot：PROT_READ、PROT_WRITE 决定映射区能否读写
 * - flags：MAP_SHARED（修改写回文件、进程间可见）与 MAP_PRIVATE（写时复制，修改不影响文件）
 * - msync：把 MAP_SHARED 的修改刷到磁盘
 * - munmap / close：映射与 fd 的生命周期相互独立
 *
 * mmap 的核心思想：把文件"贴"到进程的虚拟地址空间，之后像访问数组一样访问文件，
 *                不再需要 read/write 把数据在内核 page cache 与用户缓冲区之间来回拷贝。
 * @par 实际应用
 * - 大文件加载：游戏资源包、机器学习模型权重、字典文件，mmap 后按需加载，
 *              只有真正被访问的页才会读入内存，启动快、省内存。
 * - 数据库存储引擎：LMDB、SQLite（mmap 模式）、MongoDB 早期的 MMAPv1，
 *                直接把数据文件映射进内存，用指针读写记录。
 * - 动态链接库加载：.so / .dylib 的代码段就是用 MAP_PRIVATE 映射的，
 *                 多个进程共享同一份物理页，各自的修改（如重定位）通过 COW 互不影响。
 * - 日志 / 持久化队列：用 MAP_SHARED 映射日志文件，写内存即写文件，
 *                   如微信 Mars xlog、腾讯 MMKV，即使进程崩溃数据也已在 page cache 中不会丢失。
 */
#pragma once

namespace mmap_learn {

/**
 * @brief 阶段 1 演示集合：mmap 读文件、MAP_SHARED 写文件、SHARED 与 PRIVATE 对比。
 *
 * 所有 demo 都是静态函数，通过 run_all() 依次执行；临时文件放在 /tmp 下。
 */
class Stage1Basic {
public:
    /**
     * @brief Demo 1：用只读 mmap 读取文件内容并打印。
     *
     * 流程：先用传统 pwrite 创建测试文件
     *      → 以 PROT_READ + MAP_PRIVATE 映射
     *      → 把映射地址当作 char 数组直接打印
     *      → 离开作用域自动 munmap + close。
     *
     * @par 实际应用
     * 只读映射是最常见的 mmap 用法，适合"读多写少、随机访问"的大文件：
     * - 加载只读资源（字体、图片资源包、词典、模型文件），无需一次性 read 进内存；
     * - 文本搜索工具（如 ripgrep 对大文件会使用 mmap）直接在映射区上匹配，省去拷贝；
     * - 多个进程映射同一个文件时，共享 page cache 中的同一份物理页，内存只占一份。
     *
     * @note 按需加载：mmap 调用本身只建立虚拟地址映射，并不读文件；
     *       第一次访问某页时触发缺页中断（page fault），内核才把这一页从磁盘读入 page cache。
     * @note 映射长度不能为 0，因此空文件不能直接 mmap。
     * @throw std::runtime_error 打开文件或 mmap 失败时由 FileDescriptor / MmapRegion 抛出
     */
    static void demo_read_file();

    /**
     * @brief Demo 2：用 MAP_SHARED + PROT_WRITE 映射文件，直接写内存来修改文件。
     *
     * 流程：创建内容为 "ABCDEFGH" 的文件 → 以读写 + 共享方式映射 →
     * 通过指针改前 4 个字节为 "XYZW" → msync 刷盘 → 用传统 read() 验证文件已被修改。
     *
     * @par 实际应用
     * MAP_SHARED 让"写内存"等价于"写文件"，适合需要频繁、小块、随机写入的持久化场景：
     * - 键值存储：MMKV（微信的 key-value 组件）用 mmap 文件保存配置，写入几乎是内存速度；
     * - 高性能日志：xlog 等日志库把日志写进 mmap 缓冲区，进程崩溃时数据已在 page cache，
     *   由内核负责写回磁盘，不会丢失；
     * - 数据库原地更新：LMDB 直接修改映射页中的 B+ 树节点。
     * 相比 write()：省掉了用户缓冲区到内核的拷贝，也省掉了每次写入的系统调用开销。
     *
     * @note 写入映射区后数据先进入 page cache（成为"脏页"），内核会在之后择机写回磁盘；
     *       调用 msync(MS_SYNC) 可以强制立即落盘，相当于 write 之后的 fsync。
     * @note 打开文件时必须带写权限（O_RDWR），否则 MAP_SHARED + PROT_WRITE 会失败（EACCES）。
     * @throw std::runtime_error 打开文件或 mmap 失败时由 FileDescriptor / MmapRegion 抛出
     */
    static void demo_write_shared();

    /**
     * @brief Demo 3：对比 MAP_SHARED 与 MAP_PRIVATE 写入后对文件的影响。
     *
     * - Part A（MAP_SHARED）：把映射区改成 "BBBB"，磁盘文件随之变为 "BBBB"；
     * - Part B（MAP_PRIVATE）：把映射区改成 "CCCC"，磁盘文件仍是 "AAAA"。
     *
     * @par 实际应用
     * MAP_PRIVATE 的写时复制（COW）用于"以文件为初始内容，但修改只属于自己"的场景：
     * - 程序加载：可执行文件和动态库的数据段用 MAP_PRIVATE 映射，
     *   全局变量被修改时只复制那一页，不会改动磁盘上的 .so 文件，也不影响其他进程；
     * - 读取配置/模板后在内存中就地修改，又不希望写回原文件；
     * - fork 之后父子进程的内存也是同样的 COW 机制。
     *
     * MAP_SHARED 则用于"修改要持久化或让其他进程看到"的场景（见 Demo 2）。
     *
     * @note COW（Copy-On-Write）：MAP_PRIVATE 映射一开始与 page cache 共享物理页；
     *       第一次写某页时触发缺页，内核复制出一份私有页给本进程，之后的写入只落在副本上。
     * @note MAP_PRIVATE 即使用 PROT_WRITE 映射，也只需要以 O_RDONLY 打开文件，
     *       因为写入永远不会回到文件（Part B 正是这样用的）。
     * @throw std::runtime_error 打开文件或 mmap 失败时由 FileDescriptor / MmapRegion 抛出
     */
    static void demo_shared_vs_private();

    /**
     * @brief 依次运行阶段 1 的全部 demo。
     *
     * 由 main.cpp 的菜单项 1 或命令行参数 `./mmap_learn 1` 调用。
     */
    static void run_all();
};

} // namespace mmap_learn
