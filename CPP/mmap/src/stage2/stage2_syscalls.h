/**
 * @file stage2_syscalls.h
 * @brief 阶段 2：mmap 的配套系统调用 —— msync / mprotect / madvise / mlock / ftruncate。
 *
 * mmap 只负责"建立映射"，映射建立之后的刷盘、改权限、访问提示、锁页等
 * 都要靠配套系统调用完成。本阶段对应的知识点：
 * - msync     : 控制 MAP_SHARED 脏页何时真正落盘
 * - mprotect  : 运行时修改页权限，非法访问触发 SIGSEGV
 * - madvise   : 告诉内核访问模式，影响预读与页回收
 * - mlock     : 把页锁在物理内存里，防止被换出到 swap
 * - ftruncate : 映射前先设置文件大小；访问超出文件末尾的页触发 SIGBUS
 *
 * @par 实际应用
 * - 数据库保证持久性：LMDB 在事务提交时调用 msync，确保数据已写入磁盘再返回成功；
 * - JIT 编译器（V8、JVM、LuaJIT）：先以 RW 写入机器码，再 mprotect 成 RX 执行，实现 W^X 安全策略；
 * - 内存调试 / 越界检测：Electric Fence、guard page 在缓冲区后放一页 PROT_NONE，越界即崩溃；
 * - 大文件顺序处理：视频转码、日志分析对映射区调用 MADV_SEQUENTIAL，让内核加大预读；
 * - 密码学库（OpenSSL、libsodium）：用 mlock 防止密钥被换出到磁盘；
 * - 可增长的映射文件（MMKV、LMDB）：空间不足时先 ftruncate 扩大文件，再重新映射。
 */
#pragma once

#include "../mmap_wrapper.hpp"

#include <csetjmp>
#include <csignal>
#include <cstddef>

namespace mmap_learn {

/**
 * @brief madvise 访问策略（Strategy 模式），由 Stage2Syscalls::apply_madvise_strategy 转换为 MADV_* 常量。
 *
 * @par 实际应用
 * 同一份映射在不同阶段可以切换策略：例如数据库全表扫描时用 Sequential，
 * 索引点查时用 Random，扫描结束后用 DontNeed 释放内存。
 *
 * @note madvise 只是"建议"，内核可以忽略；具体效果因系统而异。
 */
enum class MadviseStrategy {
    Sequential, ///< MADV_SEQUENTIAL：顺序访问，内核加大预读，已读过的页可尽快回收
    Random,     ///< MADV_RANDOM    ：随机访问，内核减少或关闭预读，避免读入用不到的页
    WillNeed,   ///< MADV_WILLNEED  ：即将访问，内核提前异步把页读入 page cache
    DontNeed,   ///< MADV_DONTNEED  ：暂时不需要，内核可回收这些页（Linux 上私有匿名页再访问时为 0）
    Normal,     ///< MADV_NORMAL    ：默认行为，不做特殊处理
};

/**
 * @brief 阶段 2 演示集合：演示 mmap 配套系统调用的用法与常见陷阱。
 *
 * 包含 5 个 demo（msync、mprotect、madvise、mlock、ftruncate + SIGBUS），
 * 以及一个 madvise 策略应用函数，通过 run_all() 依次执行。
 *
 * 其中 mprotect 和 SIGBUS 两个 demo 会"故意"触发信号，
 * 借助 sigsetjmp / siglongjmp 从信号处理函数跳回，避免进程直接崩溃。
 */
class Stage2Syscalls {
    static sigjmp_buf s_jump_buf_;                  ///< sigsetjmp 保存的跳转点，信号处理函数通过它跳回
    static volatile sig_atomic_t s_signal_caught_;  ///< 捕获到的信号编号，0 表示未捕获

    /**
     * @brief SIGSEGV / SIGBUS 的信号处理函数：记录信号编号后跳回 sigsetjmp 处。
     * @param sig 收到的信号编号
     * @note 访问非法内存后若直接从处理函数 return，CPU 会重新执行那条指令并再次触发信号，
     *       形成死循环，因此必须用 siglongjmp 跳过出错的指令。
     */
    static void sigsegv_handler(int sig);

    /**
     * @brief 为指定信号安装 sigsegv_handler，并保存原来的处理方式。
     * @param signum     信号编号，如 SIGSEGV、SIGBUS
     * @param old_action 输出参数，保存原处理方式，供 restore_signal_handler 恢复
     */
    static void install_signal_handler(int signum, struct sigaction &old_action);

    /**
     * @brief 恢复信号原来的处理方式。
     * @param signum     信号编号
     * @param old_action install_signal_handler 保存的原处理方式
     */
    static void restore_signal_handler(int signum, const struct sigaction &old_action);

    /**
     * @brief RAII 临时文件：构造时 mkstemp 创建，析构时 close + unlink。
     *
     * 保证 demo 中途 return 或 MmapRegion 构造抛异常时，fd 不泄漏、临时文件被删除。
     *
     * @code
     * TempFile tmp("msync");
     * if (!tmp.valid()) return;
     * MmapRegion region(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, tmp.fd(), 0);
     * @endcode
     *
     * @note 在 TempFile 之后声明的 MmapRegion 会先析构（先 munmap 再 close），
     *       顺序反过来也没问题：close 之后映射依然有效。
     */
    class TempFile {
        int fd_ = -1;         ///< mkstemp 返回的 fd，-1 表示创建失败
        char path_[256] = {}; ///< 生成的完整路径，析构时据此 unlink

    public:
        /**
         * @brief 用 mkstemp 在工作目录 kWorkDir（mmap_tmp/）下创建唯一命名的临时文件并以读写方式打开。
         * @param suffix 文件名中间部分，生成形如 mmap_tmp/mmap_learn_<suffix>_XXXXXX 的路径
         * @note 失败时打印错误信息，valid() 返回 false，不抛异常。
         */
        explicit TempFile(const char *suffix);

        /** @brief 关闭 fd 并删除临时文件（创建失败时什么也不做）。 */
        ~TempFile();

        /** @brief 禁止拷贝，避免同一个文件被 close / unlink 两次。 */
        TempFile(const TempFile &) = delete;

        /** @brief 禁止拷贝赋值，理由同上。 */
        TempFile &operator=(const TempFile &) = delete;

        /**
         * @brief 是否创建成功。
         * @return 持有有效 fd 时返回 true
         */
        bool valid() const;

        /**
         * @brief 获取原始 fd，传给 ftruncate / mmap / read 等，不转移所有权。
         * @return 文件描述符，失败时为 -1
         */
        int fd() const;

        /**
         * @brief 获取临时文件的完整路径。
         * @return 以 '\0' 结尾的路径字符串，生命周期与本对象相同
         */
        const char *path() const;
    };

public:
    /**
     * @brief 把 MadviseStrategy 转换为对应的 MADV_* 常量，并对整个映射区调用 madvise。
     *
     * @par 实际应用
     * 把"访问模式"抽象成枚举，业务代码按场景选择策略即可，不用记 MADV_* 常量；
     * 例如文件扫描器在开始时设为 Sequential，查找器设为 Random。
     *
     * @param region   要设置访问提示的映射区域
     * @param strategy 访问策略
     * @return madvise 的返回值：成功 0，失败 -1 并设置 errno
     */
    static int apply_madvise_strategy(MmapRegion &region, MadviseStrategy strategy);

    /**
     * @brief Demo 1：msync —— 把 MAP_SHARED 映射中的修改强制写回磁盘。
     *
     * 流程：创建临时文件并 ftruncate 到一页 → 共享读写映射 → memcpy 写入字符串 →
     * msync(MS_SYNC) 刷盘 → 用 read() 从同一个 fd 读回验证。
     *
     * @par 实际应用
     * 写入映射区只保证数据进了 page cache，掉电或系统崩溃时仍可能丢失。
     * 需要"确认已持久化"的场景必须调用 msync：
     * - 数据库事务提交（LMDB 的 mdb_env_sync）；
     * - 写完关键配置、checkpoint 后需要保证掉电不丢；
     * - 只需尽快开始写回、不想阻塞时用 MS_ASYNC。
     *
     * @note 进程崩溃不会丢数据（脏页在内核中，内核会写回）；只有内核崩溃或掉电才会丢，
     *       msync 防的是后者。
     * @note 同一文件的 read() 与 MAP_SHARED 映射共用 page cache，所以即使不 msync，
     *       read() 也能立刻读到映射区的修改；msync 关心的是"磁盘"而非"可见性"。
     */
    static void demo_msync();

    /**
     * @brief Demo 2：mprotect —— 运行时把映射区改为只读，再写入时捕获 SIGSEGV。
     *
     * 流程：匿名读写映射一页 → 写入数据 → mprotect(PROT_READ) →
     * 安装信号处理函数后故意写入 → 捕获信号 → 恢复处理函数。
     * 不需要把权限改回读写再析构：munmap 与页权限无关，只读甚至 PROT_NONE 的映射都能直接解除。
     *
     * @par 实际应用
     * - JIT 的 W^X：先 PROT_READ|PROT_WRITE 写机器码，再改成 PROT_READ|PROT_EXEC 执行，
     *   防止攻击者往可执行内存里注入代码（macOS 上更是强制要求）；
     * - Guard page：线程栈底、内存池块之间放一页 PROT_NONE，越界访问立即崩溃而不是悄悄写坏数据；
     * - 保护只读数据：初始化完成后把配置表、常量表设为只读，防止被误改；
     * - 用户态缺页处理：GC 写屏障、数据库快照、分布式共享内存用 mprotect + SIGSEGV 追踪"哪些页被写过"。
     *
     * @note mprotect 的地址必须按页对齐，权限以页为单位生效。
     * @note 写只读页时 Linux 发送 SIGSEGV；macOS 上可能发送 SIGBUS，所以两个信号都安装了处理函数。
     */
    static void demo_mprotect();

    /**
     * @brief Demo 3：madvise —— 告诉内核接下来的访问模式（顺序 / 即将访问）。
     *
     * 流程：创建 1MB 文件并映射 → MADV_SEQUENTIAL 后按页顺序扫描（每页读 1 字节）→
     * MADV_WILLNEED 让内核提前把页读入内存。
     *
     * @par 实际应用
     * 内核默认的预读策略不一定适合你的访问模式，madvise 可以显著影响 I/O 性能：
     * - 顺序处理大文件（日志分析、视频处理、备份校验）：MADV_SEQUENTIAL 加大预读，读过的页尽快回收；
     * - 随机点查（数据库索引、哈希表文件）：MADV_RANDOM 关闭预读，避免读入大量无用的页；
     * - 已知马上要用（游戏切换关卡前预加载资源）：MADV_WILLNEED 提前异步加载，减少卡顿；
     * - 用完的大块缓存：MADV_DONTNEED 归还物理内存，glibc / jemalloc 释放内存时就用它。
     *
     * @note madvise 只是建议，不改变程序语义（MADV_DONTNEED 例外，见 MadviseStrategy::DontNeed）。
     * @note macOS 上 MADV_DONTNEED 只是提示，不会像 Linux 那样立即丢弃页内容。
     */
    static void demo_madvise();

    /**
     * @brief Demo 4：mlock / munlock —— 把页锁定在物理内存中，禁止换出到 swap。
     *
     * 流程：匿名映射一页 → mlock 锁定 → 写入"敏感数据" → munlock 解锁。
     *
     * @par 实际应用
     * - 安全：密码、私钥、会话密钥如果被换出到 swap 分区，就会以明文留在磁盘上，
     *   OpenSSL、libsodium（sodium_mlock）、GnuPG 都会 mlock 存放密钥的内存；
     * - 实时 / 低延迟系统：音频处理、高频交易、工业控制用 mlock（或 mlockall）
     *   避免关键路径上出现缺页导致的毫秒级抖动；
     * - 配合 MAP_POPULATE / MAP_LOCKED 在启动时一次性把内存准备好。
     *
     * @note 可锁定的内存量受 RLIMIT_MEMLOCK 限制（`ulimit -l` 查看），普通用户通常只有几十 KB 到几 MB；
     *       超出时 mlock 返回 ENOMEM 或 EPERM。
     * @note mlock 只防止换出，不会自动清零；存放密钥的内存释放前还应手动擦除。
     */
    static void demo_mlock();

    /**
     * @brief Demo 5：ftruncate 与 SIGBUS 陷阱 —— 访问超出文件末尾的映射页会触发 SIGBUS。
     *
     * 流程：创建只有 16 字节的文件 → 映射 2 页 → 写偏移 0（在文件范围内，正常）→
     * 读偏移 page_size（第二页完全超出文件，触发 SIGBUS）→ 捕获信号并清理。
     * SIGBUS 和 SIGSEGV 都安装了处理函数（个别平台对这类越界访问发送 SIGSEGV），结束后两者都恢复。
     *
     * @par 实际应用
     * 这是 mmap 最常见的线上崩溃原因之一，理解它才能写出健壮的代码：
     * - 写新文件前必须先 ftruncate 到目标大小，再映射写入（MMKV、LMDB 扩容都是这样做的）；
     * - 映射期间文件被其他进程截断（如日志轮转把文件清空），本进程再访问就会 SIGBUS，
     *   需要用文件锁或约定避免；
     * - 文件位于 NFS / 可移动存储时发生 I/O 错误，mmap 无法像 read() 那样返回 -1，
     *   只能以 SIGBUS 的形式出现。
     *
     * @note 文件最后一页中超出文件大小的部分（这里是第一页的 16 字节之后）可以访问，读到的是 0，
     *       写入也不会改变文件长度；只有完全落在文件之外的页才会 SIGBUS。
     * @note mmap 调用本身不检查文件大小，映射总是成功的，错误要到访问时才暴露，
     *       所以这里直接用 ::mmap 而不是 MmapRegion 来演示。
     */
    static void demo_ftruncate_sigbus();

    /**
     * @brief 依次运行阶段 2 的全部 demo。
     *
     * 由 main.cpp 的菜单项 2 或命令行参数 `./mmap_learn 2` 调用。
     */
    static void run_all();
};

} // namespace mmap_learn
