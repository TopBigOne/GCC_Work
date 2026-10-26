#include "stage2_syscalls.h"

#include <cerrno>
#include <csetjmp>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace mmap_learn {

    // 静态成员变量的定义（.h 里只是声明）。
    // 信号处理函数是普通函数指针，拿不到 this，只能通过静态/全局变量和 demo 交流
    sigjmp_buf Stage2Syscalls::s_jump_buf_;                     // sigsetjmp 的"存档"，处理函数从这里跳回
    volatile sig_atomic_t Stage2Syscalls::s_signal_caught_ = 0; // 记录收到的信号编号；0 表示还没收到

    void Stage2Syscalls::sigsegv_handler(int sig) {
        // 信号处理函数里只能做异步信号安全的事：写一个 volatile sig_atomic_t，然后跳走。
        // printf、malloc 都不安全，打印留到跳回后的 else 分支里做
        s_signal_caught_ = sig;

        // 不能直接 return：SIGSEGV/SIGBUS 返回后 CPU 会重新执行出错的那条指令，
        // 再次出错，陷入死循环。所以用 siglongjmp 跳回 demo 里 sigsetjmp 的位置，
        // 让 sigsetjmp 第二次返回，返回值为 1
        siglongjmp(s_jump_buf_, 1);
    }

    void Stage2Syscalls::install_signal_handler(int signum, struct sigaction &old_action) {
        struct sigaction sa{};
        sa.sa_handler = sigsegv_handler; // 收到 signum 时调用的函数
        sigemptyset(&sa.sa_mask);        // 处理函数执行期间不额外阻塞其他信号
                                         // （正在处理的这个信号会被内核自动阻塞）
        sa.sa_flags = 0;                 // 不用 SA_SIGINFO：只需要信号编号，不需要出错地址

        // 安装新的处理方式，同时把原来的处理方式保存到 old_action，demo 结束后还原
        sigaction(signum, &sa, &old_action);
    }

    void Stage2Syscalls::restore_signal_handler(int signum, const struct sigaction &old_action) {
        // sigaction 改的是整个进程的信号处理方式。用完必须还原，
        // 否则之后真正的崩溃也会被这个处理函数"吞掉"，跳回一个早已失效的存档点
        sigaction(signum, &old_action, nullptr);
    }

    Stage2Syscalls::TempFile::TempFile(const char *suffix) {
        // 生成文件名模板，如 mmap_tmp/mmap_learn_msync_XXXXXX（kWorkDir 见 mmap_wrapper.hpp）。
        // 末尾必须正好是 6 个 X，mkstemp 会把它们原地替换成随机字符
        snprintf(path_, sizeof(path_), "%s/mmap_learn_%s_XXXXXX", kWorkDir, suffix);

        // mkstemp 一步完成"生成唯一文件名 + 创建并以读写方式打开"，
        // 用 O_EXCL 保证文件是新建的，不会和别人的文件撞名，也不会被抢先创建（竞态安全）。
        // 成功后 path_ 里就是最终的文件名，返回值是打开的 fd
        fd_ = mkstemp(path_);
        if (fd_ < 0) {
            // 失败不抛异常，只打印错误；调用方通过 valid() 判断后自行返回
            printf("[ERROR] mkstemp failed: %s\n", strerror(errno));
        }
    }

    Stage2Syscalls::TempFile::~TempFile() {
        if (fd_ >= 0) {
            // 关闭文件描述符
            close(fd_);

            // 删除文件名。unlink 只删目录里的名字，若此时还有映射在用这个文件，
            // 数据会保留到映射解除为止。demo 里 TempFile 声明在 MmapRegion 之前，
            // 析构顺序相反，所以走到这里时映射已经先被 munmap 了
            unlink(path_);
        }
    }

    bool Stage2Syscalls::TempFile::valid() const { return fd_ >= 0; }

    int Stage2Syscalls::TempFile::fd() const { return fd_; }

    const char *Stage2Syscalls::TempFile::path() const { return path_; }

    int Stage2Syscalls::apply_madvise_strategy(MmapRegion &region, MadviseStrategy strategy) {
        int advice = MADV_NORMAL;
        const char *name = "MADV_NORMAL";

        switch (strategy) {
            case MadviseStrategy::Sequential:
                advice = MADV_SEQUENTIAL;
                name = "MADV_SEQUENTIAL";
                break;
            case MadviseStrategy::Random:
                advice = MADV_RANDOM;
                name = "MADV_RANDOM";
                break;
            case MadviseStrategy::WillNeed:
                advice = MADV_WILLNEED;
                name = "MADV_WILLNEED";
                break;
            case MadviseStrategy::DontNeed:
                advice = MADV_DONTNEED;
                name = "MADV_DONTNEED";
                break;
            case MadviseStrategy::Normal:
                advice = MADV_NORMAL;
                name = "MADV_NORMAL";
                break;
        }

        printf("  Applying madvise strategy: %s\n", name);
        int ret = region.advise(advice);
        if (ret < 0) {
            printf("  [ERROR] madvise(%s) failed: %s\n", name, strerror(errno));
        } else {
            printf("  madvise(%s) succeeded\n", name);
        }
        return ret;
    }

    /**
     * 把 MAP_SHARED 映射中的修改强制写回磁盘。
     */
    void Stage2Syscalls::demo_msync() {
        printf("=== Demo: msync ===\n");

        // RAII 临时文件：任何路径退出（含异常）都会 close + unlink
        TempFile tmp("msync");
        if (!tmp.valid())
            return;
        printf("  Created temp file: %s\n", tmp.path());

        const char *message = "Hello from mmap + msync!";
        size_t msg_len = strlen(message);
        size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));

        // 先把文件扩展到一页大小，保证映射区有文件内容作为后备存储，否则访问会 SIGBUS
        if (ftruncate(tmp.fd(), static_cast<off_t>(page_size)) < 0) {
            printf("  [ERROR] ftruncate failed: %s\n", strerror(errno));
            return;
        }

        // 共享 + 读写映射
        MmapRegion region(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, tmp.fd(), 0);
        printf("  mmap'd %zu bytes (SHARED | RW)\n", page_size);

        // 向映射区写入数据（连同结尾的 '\0'）
        memcpy(region.data(), message, msg_len + 1);
        printf("  Wrote to mmap region: \"%s\"\n", message);

        // msync 同步刷盘
        int ret = region.sync(MS_SYNC);
        if (ret < 0) {
            printf("  [ERROR] msync failed: %s\n", strerror(errno));
        } else {
            printf("  msync(MS_SYNC) succeeded - data flushed to disk\n");
        }

        // 用 read() 读回文件验证
        lseek(tmp.fd(), 0, SEEK_SET);
        char verify_buf[256] = {};
        ssize_t n = read(tmp.fd(), verify_buf, sizeof(verify_buf) - 1);
        if (n > 0) {
            printf("  Verified via read(): \"%s\"\n", verify_buf);
        } else {
            printf("  [ERROR] read() verification failed: %s\n", strerror(errno));
        }

        // 函数结束时按声明的逆序析构：region 先 munmap，tmp 再 close + unlink
        printf("  Cleaned up temp file\n\n");
    }

    /**
     * 运行时把映射区改为只读，再写入时捕获 SIGSEGV。
     */
    void Stage2Syscalls::demo_mprotect() {
        printf("=== Demo: mprotect ===\n");

        size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));

        // 匿名读写映射一页
        MmapRegion region(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        printf("  mmap'd anonymous region: %zu bytes (RW)\n", page_size);

        // 可写时先写入数据
        const char *msg = "writable data";
        memcpy(region.data(), msg, strlen(msg) + 1);
        printf("  Wrote data while region is RW: \"%s\"\n", region.as<char>());

        // 改为只读
        int ret = region.protect(PROT_READ);
        if (ret < 0) {
            printf("  [ERROR] mprotect(PROT_READ) failed: %s\n", strerror(errno));
            return;
        }
        printf("  mprotect(PROT_READ) succeeded - region is now read-only\n");

        // 安装 SIGSEGV 处理函数（同时装 SIGBUS，兼容 macOS）
        struct sigaction old_segv{}, old_bus{};
        install_signal_handler(SIGSEGV, old_segv);
        install_signal_handler(SIGBUS, old_bus);

        // 尝试写入只读区域
        printf("  Attempting write to read-only region...\n");
        s_signal_caught_ = 0;

        // sigsetjmp 第一次调用返回 0；信号处理函数 siglongjmp 跳回时返回 1
        if (sigsetjmp(s_jump_buf_, 1) == 0) {
            // 第一次返回：执行非法写入。volatile 防止编译器把这次写入优化掉
            volatile char *p = static_cast<volatile char *>(region.data());
            *p = 'X'; // 应触发 SIGSEGV
            printf("  [UNEXPECTED] Write succeeded (should not happen)\n");
        } else {
            // 从信号处理函数 siglongjmp 跳回到这里
            printf("  Caught SIGBUS/SIGSEGV: write to read-only mmap region (signal=%d)\n", s_signal_caught_);
        }

        // 恢复原来的信号处理方式
        restore_signal_handler(SIGSEGV, old_segv);
        restore_signal_handler(SIGBUS, old_bus);

        // 不需要把权限改回 RW：munmap 与页权限无关，只读映射也能直接解除，
        // region 析构时会正常 munmap
        printf("  Region stays read-only; munmap does not need write permission\n\n");
    }


    /**
     * 告诉内核接下来的访问模式（顺序 / 即将访问）。
     */
    void Stage2Syscalls::demo_madvise() {
        printf("=== Demo: madvise ===\n");

        // RAII 临时文件：任何路径退出（含异常）都会 close + unlink
        TempFile tmp("madvise");
        if (!tmp.valid())
            return;

        // 创建 1MB 的文件
        constexpr size_t file_size = 1024 * 1024; // 1 MB
        if (ftruncate(tmp.fd(), static_cast<off_t>(file_size)) < 0) {
            printf("  [ERROR] ftruncate failed: %s\n", strerror(errno));
            return;
        }
        printf("  Created 1MB temp file: %s\n", tmp.path());

        // 映射整个文件
        MmapRegion region(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, tmp.fd(), 0);
        printf("  mmap'd %zu bytes\n", file_size);

        // 演示 MADV_SEQUENTIAL
        printf("\n  [MADV_SEQUENTIAL]\n");
        printf("  Tells the kernel: expect sequential page references.\n");
        printf("  Effect: aggressive read-ahead, free pages behind the cursor.\n");
        apply_madvise_strategy(region, MadviseStrategy::Sequential);

        // 模拟顺序访问：每隔一页读 1 个字节，每次都会触发一次缺页
        // sum 声明为 volatile，防止编译器把整个循环优化掉
        volatile unsigned char sum = 0;
        const unsigned char *data = region.as<const unsigned char>();
        for (size_t i = 0; i < file_size; i += 4096) {
            sum += data[i];
        }
        printf("  Sequential scan complete (checksum byte: %u)\n", static_cast<unsigned>(sum));

        // 演示 MADV_WILLNEED
        printf("\n  [MADV_WILLNEED]\n");
        printf("  Tells the kernel: these pages will be needed soon.\n");
        printf("  Effect: initiate read-ahead to populate page cache.\n");
        apply_madvise_strategy(region, MadviseStrategy::WillNeed);

        printf("  Kernel is now pre-faulting pages into memory\n");

        // region 和 tmp 在函数结束时自动清理（munmap → close + unlink）
        printf("  Cleaned up temp file\n\n");
    }

    /**
     * 把页锁定在物理内存中，禁止换出到 swap。
     */
    void Stage2Syscalls::demo_mlock() {
        printf("=== Demo: mlock / munlock ===\n");

        size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));

        // 匿名映射一页
        MmapRegion region(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        printf("  mmap'd anonymous page: %zu bytes\n", page_size);

        // mlock：锁定在物理内存中
        int ret = region.lock();
        if (ret < 0) {
            printf("  [ERROR] mlock failed: %s (try running as root or increase RLIMIT_MEMLOCK)\n", strerror(errno));
        } else {
            printf("  mlock succeeded - page is pinned in physical RAM (no swapping)\n");
        }

        // 锁定期间写入敏感数据
        const char *secret = "sensitive-key-material";
        memcpy(region.data(), secret, strlen(secret) + 1);
        printf("  Wrote sensitive data while page is locked: \"%s\"\n", region.as<char>());

        // munlock：允许再次被换出
        ret = region.unlock();
        if (ret < 0) {
            printf("  [ERROR] munlock failed: %s\n", strerror(errno));
        } else {
            printf("  munlock succeeded - page can now be swapped out\n");
        }

        printf("  Use case: cryptographic keys, passwords - prevent swap leakage\n\n");
    }

    void Stage2Syscalls::demo_ftruncate_sigbus() {
        printf("=== Demo: ftruncate + SIGBUS trap ===\n");

        // RAII 临时文件：任何路径退出都会 close + unlink
        TempFile tmp("sigbus");
        if (!tmp.valid())
            return;

        // 创建一个很小的文件：只有 16 字节
        constexpr size_t file_size = 16;
        if (ftruncate(tmp.fd(), static_cast<off_t>(file_size)) < 0) {
            printf("  [ERROR] ftruncate failed: %s\n", strerror(errno));
            return;
        }
        printf("  Created tiny file: %s (%zu bytes)\n", tmp.path(), file_size);

        // 用这个小文件映射一块更大的区域（2 页）
        size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
        size_t map_size = page_size * 2;

        // 这里直接使用原始 mmap：映射本身会成功（MmapRegion 也一样），问题要到访问时才出现
        void *addr = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, tmp.fd(), 0);
        if (addr == MAP_FAILED) {
            printf("  [ERROR] mmap failed: %s\n", strerror(errno));
            return;
        }
        printf("  mmap'd %zu bytes (2 pages) backed by %zu-byte file\n", map_size, file_size);
        printf("  The file only covers part of the first page.\n");
        printf("  Accessing beyond file size (but within mapped region) triggers SIGBUS.\n");

        // 安装 SIGBUS 处理函数；同时装 SIGSEGV，个别平台对这类越界访问发送的是 SIGSEGV
        struct sigaction old_bus{}, old_segv{};
        install_signal_handler(SIGBUS, old_bus);
        install_signal_handler(SIGSEGV, old_segv);
        s_signal_caught_ = 0;

        // 在文件范围内访问 —— 正常
        volatile char *p = static_cast<volatile char *>(addr);
        p[0] = 'A';
        printf("  Write at offset 0: OK\n");

        // 访问第二页：整页都在文件末尾之外，大多数系统上会触发 SIGBUS
        printf("  Attempting access at offset %zu (beyond file, second page)...\n", page_size);

        if (sigsetjmp(s_jump_buf_, 1) == 0) {
            volatile char c = p[page_size]; // 访问第二页 —— SIGBUS
            (void) c;
            printf("  [NOTE] Access succeeded (kernel may have zero-filled the page)\n");
        } else {
            printf("  Caught SIGBUS/SIGSEGV: access beyond end of file in mmap region (signal=%d)\n", s_signal_caught_);
            printf("  This happens when accessing mmap'd pages beyond the backing file's size.\n");
        }

        // 恢复信号处理方式并清理（tmp 在函数结束时自动 close + unlink）
        restore_signal_handler(SIGBUS, old_bus);
        restore_signal_handler(SIGSEGV, old_segv);
        ::munmap(addr, map_size);
        printf("  Cleaned up temp file\n\n");
    }

    void Stage2Syscalls::run_all() {
        printf("################################################################\n");
        printf("# Stage 2: Companion System Calls for mmap                     #\n");
        printf("################################################################\n\n");

        demo_msync();
        demo_mprotect();
        demo_madvise();
        demo_mlock();
        demo_ftruncate_sigbus();

        printf("################################################################\n");
        printf("# Stage 2 Complete                                             #\n");
        printf("################################################################\n");
    }

} // namespace mmap_learn
