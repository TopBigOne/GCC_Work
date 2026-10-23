#pragma once

#include "mmap_wrapper.hpp"

#include <cerrno>
#include <csetjmp>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace mmap_learn {

// ============================================================
// Strategy pattern: madvise strategies
// ============================================================
enum class MadviseStrategy {
    Sequential, // MADV_SEQUENTIAL: expect sequential page references
    Random,     // MADV_RANDOM: expect random page references
    WillNeed,   // MADV_WILLNEED: will need these pages soon
    DontNeed,   // MADV_DONTNEED: don't need these pages soon
    Normal,     // MADV_NORMAL: no special treatment (default)
};

// ============================================================
// Stage 2: Companion system calls for mmap
// ============================================================
class Stage2Syscalls {
    // -- signal handling helpers (file-scope statics via inline) --
    static inline sigjmp_buf s_jump_buf_;
    static inline volatile sig_atomic_t s_signal_caught_ = 0;

    static void sigsegv_handler(int sig) {
        s_signal_caught_ = sig;
        siglongjmp(s_jump_buf_, 1);
    }

    static void install_signal_handler(int signum, struct sigaction &old_action) {
        struct sigaction sa {};
        sa.sa_handler = sigsegv_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(signum, &sa, &old_action);
    }

    static void restore_signal_handler(int signum, const struct sigaction &old_action) {
        sigaction(signum, &old_action, nullptr);
    }

    // -- temp file helper --
    static int create_temp_file(const char *suffix, char *path_buf, size_t buf_size) {
        snprintf(path_buf, buf_size, "/tmp/mmap_learn_%s_XXXXXX", suffix);
        int fd = mkstemp(path_buf);
        if (fd < 0) {
            printf("[ERROR] mkstemp failed: %s\n", strerror(errno));
        }
        return fd;
    }

public:
    // ============================================================
    // Strategy pattern: apply madvise strategy to an MmapRegion
    // ============================================================
    static int apply_madvise_strategy(MmapRegion &region, MadviseStrategy strategy) {
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

    // ============================================================
    // Demo 1: msync - flush mmap changes to disk
    // ============================================================
    static void demo_msync() {
        printf("=== Demo: msync ===\n");

        char path[256];
        int raw_fd = create_temp_file("msync", path, sizeof(path));
        if (raw_fd < 0) return;
        printf("  Created temp file: %s\n", path);

        const char *message = "Hello from mmap + msync!";
        size_t msg_len = strlen(message);
        size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));

        // Expand file to one page so mmap has backing storage
        if (ftruncate(raw_fd, static_cast<off_t>(page_size)) < 0) {
            printf("  [ERROR] ftruncate failed: %s\n", strerror(errno));
            close(raw_fd);
            unlink(path);
            return;
        }

        // mmap shared + read-write
        MmapRegion region(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, raw_fd, 0);
        printf("  mmap'd %zu bytes (SHARED | RW)\n", page_size);

        // Write data into the mapped region
        memcpy(region.data(), message, msg_len + 1);
        printf("  Wrote to mmap region: \"%s\"\n", message);

        // msync to flush
        int ret = region.sync(MS_SYNC);
        if (ret < 0) {
            printf("  [ERROR] msync failed: %s\n", strerror(errno));
        } else {
            printf("  msync(MS_SYNC) succeeded - data flushed to disk\n");
        }

        // Verify by reading the file with read()
        lseek(raw_fd, 0, SEEK_SET);
        char verify_buf[256] = {};
        ssize_t n = read(raw_fd, verify_buf, sizeof(verify_buf) - 1);
        if (n > 0) {
            printf("  Verified via read(): \"%s\"\n", verify_buf);
        } else {
            printf("  [ERROR] read() verification failed: %s\n", strerror(errno));
        }

        close(raw_fd);
        unlink(path);
        printf("  Cleaned up temp file\n\n");
    }

    // ============================================================
    // Demo 2: mprotect - change memory protection at runtime
    // ============================================================
    static void demo_mprotect() {
        printf("=== Demo: mprotect ===\n");

        size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));

        // mmap anonymous region read-write
        MmapRegion region(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        printf("  mmap'd anonymous region: %zu bytes (RW)\n", page_size);

        // Write data while writable
        const char *msg = "writable data";
        memcpy(region.data(), msg, strlen(msg) + 1);
        printf("  Wrote data while region is RW: \"%s\"\n", region.as<char>());

        // Change to read-only
        int ret = region.protect(PROT_READ);
        if (ret < 0) {
            printf("  [ERROR] mprotect(PROT_READ) failed: %s\n", strerror(errno));
            return;
        }
        printf("  mprotect(PROT_READ) succeeded - region is now read-only\n");

        // Install signal handler for SIGSEGV (and SIGBUS for completeness)
        struct sigaction old_segv {}, old_bus {};
        install_signal_handler(SIGSEGV, old_segv);
        install_signal_handler(SIGBUS, old_bus);

        // Try writing to read-only region
        printf("  Attempting write to read-only region...\n");
        s_signal_caught_ = 0;

        if (sigsetjmp(s_jump_buf_, 1) == 0) {
            // First return from sigsetjmp: attempt the illegal write
            volatile char *p = static_cast<volatile char *>(region.data());
            *p = 'X'; // Should trigger SIGSEGV
            printf("  [UNEXPECTED] Write succeeded (should not happen)\n");
        } else {
            // Returned via siglongjmp from signal handler
            printf("  Caught SIGBUS/SIGSEGV: write to read-only mmap region (signal=%d)\n", s_signal_caught_);
        }

        // Restore original handlers
        restore_signal_handler(SIGSEGV, old_segv);
        restore_signal_handler(SIGBUS, old_bus);

        // Restore write permission so MmapRegion destructor can unmap cleanly
        region.protect(PROT_READ | PROT_WRITE);
        printf("  Restored RW permissions for cleanup\n\n");
    }

    // ============================================================
    // Demo 3: madvise - advise kernel on memory usage patterns
    // ============================================================
    static void demo_madvise() {
        printf("=== Demo: madvise ===\n");

        char path[256];
        int raw_fd = create_temp_file("madvise", path, sizeof(path));
        if (raw_fd < 0) return;

        // Create a 1MB file
        constexpr size_t file_size = 1024 * 1024; // 1 MB
        if (ftruncate(raw_fd, static_cast<off_t>(file_size)) < 0) {
            printf("  [ERROR] ftruncate failed: %s\n", strerror(errno));
            close(raw_fd);
            unlink(path);
            return;
        }
        printf("  Created 1MB temp file: %s\n", path);

        // mmap the entire file
        MmapRegion region(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, raw_fd, 0);
        printf("  mmap'd %zu bytes\n", file_size);

        // Demonstrate MADV_SEQUENTIAL
        printf("\n  [MADV_SEQUENTIAL]\n");
        printf("  Tells the kernel: expect sequential page references.\n");
        printf("  Effect: aggressive read-ahead, free pages behind the cursor.\n");
        apply_madvise_strategy(region, MadviseStrategy::Sequential);

        // Simulate sequential access
        volatile unsigned char sum = 0;
        const unsigned char *data = region.as<const unsigned char>();
        for (size_t i = 0; i < file_size; i += 4096) {
            sum += data[i];
        }
        printf("  Sequential scan complete (checksum byte: %u)\n", static_cast<unsigned>(sum));

        // Demonstrate MADV_WILLNEED
        printf("\n  [MADV_WILLNEED]\n");
        printf("  Tells the kernel: these pages will be needed soon.\n");
        printf("  Effect: initiate read-ahead to populate page cache.\n");
        apply_madvise_strategy(region, MadviseStrategy::WillNeed);

        printf("  Kernel is now pre-faulting pages into memory\n");

        close(raw_fd);
        unlink(path);
        printf("  Cleaned up temp file\n\n");
    }

    // ============================================================
    // Demo 4: mlock / munlock - pin pages in physical memory
    // ============================================================
    static void demo_mlock() {
        printf("=== Demo: mlock / munlock ===\n");

        size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));

        // Anonymous mmap
        MmapRegion region(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        printf("  mmap'd anonymous page: %zu bytes\n", page_size);

        // mlock - pin in RAM
        int ret = region.lock();
        if (ret < 0) {
            printf("  [ERROR] mlock failed: %s (try running as root or increase RLIMIT_MEMLOCK)\n", strerror(errno));
        } else {
            printf("  mlock succeeded - page is pinned in physical RAM (no swapping)\n");
        }

        // Write data while locked
        const char *secret = "sensitive-key-material";
        memcpy(region.data(), secret, strlen(secret) + 1);
        printf("  Wrote sensitive data while page is locked: \"%s\"\n", region.as<char>());

        // munlock - allow swapping again
        ret = region.unlock();
        if (ret < 0) {
            printf("  [ERROR] munlock failed: %s\n", strerror(errno));
        } else {
            printf("  munlock succeeded - page can now be swapped out\n");
        }

        printf("  Use case: cryptographic keys, passwords - prevent swap leakage\n\n");
    }

    // ============================================================
    // Demo 5: ftruncate + SIGBUS trap
    // ============================================================
    static void demo_ftruncate_sigbus() {
        printf("=== Demo: ftruncate + SIGBUS trap ===\n");

        char path[256];
        int raw_fd = create_temp_file("sigbus", path, sizeof(path));
        if (raw_fd < 0) return;

        // Create a tiny file: only 16 bytes
        constexpr size_t file_size = 16;
        if (ftruncate(raw_fd, static_cast<off_t>(file_size)) < 0) {
            printf("  [ERROR] ftruncate failed: %s\n", strerror(errno));
            close(raw_fd);
            unlink(path);
            return;
        }
        printf("  Created tiny file: %s (%zu bytes)\n", path, file_size);

        // mmap a larger region (2 pages) backed by this tiny file
        size_t page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
        size_t map_size = page_size * 2;

        // Use raw mmap here because MmapRegion would succeed for the mapping itself
        void *addr = ::mmap(nullptr, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, raw_fd, 0);
        if (addr == MAP_FAILED) {
            printf("  [ERROR] mmap failed: %s\n", strerror(errno));
            close(raw_fd);
            unlink(path);
            return;
        }
        printf("  mmap'd %zu bytes (2 pages) backed by %zu-byte file\n", map_size, file_size);
        printf("  The file only covers part of the first page.\n");
        printf("  Accessing beyond file size (but within mapped region) triggers SIGBUS.\n");

        // Install SIGBUS handler
        struct sigaction old_bus {};
        install_signal_handler(SIGBUS, old_bus);
        s_signal_caught_ = 0;

        // Access within file bounds - should work
        volatile char *p = static_cast<volatile char *>(addr);
        p[0] = 'A';
        printf("  Write at offset 0: OK\n");

        // Access beyond file size on a page boundary past the file's last page
        // On most systems, accessing the second page (entirely beyond file) triggers SIGBUS
        printf("  Attempting access at offset %zu (beyond file, second page)...\n", page_size);

        if (sigsetjmp(s_jump_buf_, 1) == 0) {
            volatile char c = p[page_size]; // Access second page - SIGBUS
            (void)c;
            printf("  [NOTE] Access succeeded (kernel may have zero-filled the page)\n");
        } else {
            printf("  Caught SIGBUS/SIGSEGV: write to read-only mmap region (signal=%d)\n", s_signal_caught_);
            printf("  This happens when accessing mmap'd pages beyond the backing file's size.\n");
        }

        // Restore handler and cleanup
        restore_signal_handler(SIGBUS, old_bus);
        ::munmap(addr, map_size);
        close(raw_fd);
        unlink(path);
        printf("  Cleaned up temp file\n\n");
    }

    // ============================================================
    // Run all demos
    // ============================================================
    static void run_all() {
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
};

} // namespace mmap_learn
