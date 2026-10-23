#pragma once

#include "mmap_wrapper.hpp"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace mmap_learn {

class Stage1Basic {
public:
    // ============================================================
    // Demo 1: Read a file using mmap and print contents
    // ============================================================
    static void demo_read_file() {
        printf("[Stage1] demo_read_file: Read a file via mmap\n");

        const char *path = "/tmp/mmap_learn_read.txt";
        const char *message = "Hello mmap world!";
        size_t len = strlen(message);

        // Create a temp file and write content with traditional I/O
        {
            FileDescriptor fd(path, O_CREAT | O_RDWR | O_TRUNC);
            fd.truncate(static_cast<off_t>(len));
            ::pwrite(fd.get(), message, len, 0);
        }

        // mmap the file read-only and print contents
        {
            FileDescriptor fd(path, O_RDONLY);
            off_t file_size = fd.size();

            MmapRegion region = MmapBuilder()
                                    .length(static_cast<size_t>(file_size))
                                    .readOnly()
                                    .priv()
                                    .file(fd.get())
                                    .build();

            printf("  mmap addr  : %p\n", region.data());
            printf("  mmap size  : %zu\n", region.size());
            printf("  content    : %.*s\n", static_cast<int>(region.size()), region.as<const char>());
        }
        // MmapRegion destructor calls munmap; FileDescriptor destructor calls close

        printf("  RAII cleanup done (munmap + close)\n\n");
    }

    // ============================================================
    // Demo 2: Write to a file using MAP_SHARED + PROT_WRITE
    // ============================================================
    static void demo_write_shared() {
        printf("[Stage1] demo_write_shared: Write via MAP_SHARED\n");

        const char *path = "/tmp/mmap_learn_write.txt";
        const char *initial = "ABCDEFGH";
        size_t len = strlen(initial);

        // Create temp file with initial content
        {
            FileDescriptor fd(path, O_CREAT | O_RDWR | O_TRUNC);
            fd.truncate(static_cast<off_t>(len));
            ::pwrite(fd.get(), initial, len, 0);
        }

        // mmap shared + rw, modify bytes, sync
        {
            FileDescriptor fd(path, O_RDWR);
            off_t file_size = fd.size();

            MmapRegion region = MmapBuilder()
                                    .length(static_cast<size_t>(file_size))
                                    .readWrite()
                                    .shared()
                                    .file(fd.get())
                                    .build();

            printf("  before modify: %.*s\n", static_cast<int>(region.size()), region.as<const char>());

            // Overwrite first 4 bytes
            char *ptr = region.as<char>();
            ptr[0] = 'X';
            ptr[1] = 'Y';
            ptr[2] = 'Z';
            ptr[3] = 'W';

            printf("  after modify : %.*s\n", static_cast<int>(region.size()), region.as<const char>());

            // Flush changes to disk
            region.sync(MS_SYNC);
            printf("  msync done\n");
        }

        // Verify with traditional read()
        {
            FileDescriptor fd(path, O_RDONLY);
            char buf[64] = {};
            ssize_t n = ::read(fd.get(), buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("  read() verify: %s\n", buf);
            }
        }

        printf("\n");
    }

    // ============================================================
    // Demo 3: Compare MAP_SHARED vs MAP_PRIVATE (COW)
    // ============================================================
    static void demo_shared_vs_private() {
        printf("[Stage1] demo_shared_vs_private: SHARED vs PRIVATE (COW)\n");

        const char *path = "/tmp/mmap_learn_cow.txt";
        const char *original = "AAAA";
        size_t len = strlen(original);

        auto write_file = [&](const char *content) {
            FileDescriptor fd(path, O_CREAT | O_RDWR | O_TRUNC);
            fd.truncate(static_cast<off_t>(len));
            ::pwrite(fd.get(), content, len, 0);
        };

        auto read_file = [&](char *buf, size_t bufsize) {
            FileDescriptor fd(path, O_RDONLY);
            ssize_t n = ::read(fd.get(), buf, bufsize - 1);
            if (n > 0) buf[n] = '\0';
            else buf[0] = '\0';
        };

        // --- Part A: MAP_SHARED modifies the file ---
        printf("  --- MAP_SHARED ---\n");
        write_file(original);

        {
            FileDescriptor fd(path, O_RDWR);

            MmapRegion region = MmapBuilder()
                                    .length(len)
                                    .readWrite()
                                    .shared()
                                    .file(fd.get())
                                    .build();

            printf("  mmap content before: %.*s\n", static_cast<int>(len), region.as<const char>());

            // Modify to "BBBB"
            char *ptr = region.as<char>();
            memset(ptr, 'B', len);
            region.sync(MS_SYNC);

            printf("  mmap content after : %.*s\n", static_cast<int>(len), region.as<const char>());
        }

        {
            char buf[16] = {};
            read_file(buf, sizeof(buf));
            printf("  file on disk       : %s  (file changed!)\n", buf);
        }

        // --- Part B: MAP_PRIVATE does NOT modify the file (COW) ---
        printf("  --- MAP_PRIVATE (COW) ---\n");
        write_file(original);  // Reset to "AAAA"

        {
            FileDescriptor fd(path, O_RDONLY);

            MmapRegion region = MmapBuilder()
                                    .length(len)
                                    .readWrite()
                                    .priv()
                                    .file(fd.get())
                                    .build();

            printf("  mmap content before: %.*s\n", static_cast<int>(len), region.as<const char>());

            // Modify to "CCCC" -- only in our private copy
            char *ptr = region.as<char>();
            memset(ptr, 'C', len);

            printf("  mmap content after : %.*s\n", static_cast<int>(len), region.as<const char>());
        }

        {
            char buf[16] = {};
            read_file(buf, sizeof(buf));
            printf("  file on disk       : %s  (file unchanged - COW!)\n", buf);
        }

        printf("\n");
    }

    // ============================================================
    // Run all Stage 1 demos
    // ============================================================
    static void run_all() {
        printf("========================================\n");
        printf("  Stage 1: mmap Basic Operations\n");
        printf("========================================\n\n");

        demo_read_file();
        demo_write_shared();
        demo_shared_vs_private();

        printf("========================================\n");
        printf("  Stage 1 complete\n");
        printf("========================================\n");
    }
};

} // namespace mmap_learn
