#include "stage1_basic.h"

#include "../mmap_wrapper.hpp"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace mmap_learn {

void Stage1Basic::demo_read_file() {
    printf("[Stage1] demo_read_file: Read a file via mmap\n");

    const std::string path_str = work_path("mmap_learn_read.txt");
    const char *path = path_str.c_str();
    const char *message = "Hello mmap world!";
    size_t len = strlen(message);

    // 用传统 I/O 创建临时文件并写入内容
    {
        // open 的 flags 是多个标志位按位或 (|) 组合而成：
        //   O_RDWR  : 访问模式，以读写方式打开（必须在 O_RDONLY / O_WRONLY / O_RDWR 中三选一）
        //   O_CREAT : 文件不存在时创建它，权限取 FileDescriptor 的 mode 参数（默认 0644）
        //   O_TRUNC : 文件已存在时把长度截断为 0，清空上次运行留下的旧内容
        // 合起来就是"没有就新建，有就清空，然后可读可写"，保证每次 demo 都从空文件开始
        FileDescriptor fd(path, O_CREAT | O_RDWR | O_TRUNC);
        fd.truncate(static_cast<off_t>(len));
        ::pwrite(fd.get(), message, len, 0);
    }

    // 以只读方式 mmap 文件并打印内容
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
        // 映射区不以 '\0' 结尾，所以用 %.*s 指定长度打印
        printf("  content    : %.*s\n", static_cast<int>(region.size()), region.as<const char>());
    }
    // 离开作用域：MmapRegion 析构调用 munmap，FileDescriptor 析构调用 close

    printf("  RAII cleanup done (munmap + close)\n\n");
}

void Stage1Basic::demo_write_shared() {
    printf("[Stage1] demo_write_shared: Write via MAP_SHARED\n");

    const std::string path_str = work_path("mmap_learn_write.txt");
    const char *path = path_str.c_str();
    const char *initial = "ABCDEFGH";
    size_t len = strlen(initial);

    // 创建临时文件并写入初始内容
    {
        FileDescriptor fd(path, O_CREAT | O_RDWR | O_TRUNC);
        fd.truncate(static_cast<off_t>(len));
        pwrite(fd.get(), initial, len, 0);
    }

    // 共享 + 读写方式映射，修改字节，再 msync 刷盘
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

        // 直接通过指针覆盖前 4 个字节，没有任何 write() 调用
        char *ptr = region.as<char>();
        ptr[0] = 'X';
        ptr[1] = 'Y';
        ptr[2] = 'Z';
        ptr[3] = 'W';

        printf("  after modify : %.*s\n", static_cast<int>(region.size()), region.as<const char>());

        // 把脏页同步写回磁盘
        region.sync(MS_SYNC);
        printf("  msync done\n");
    }

    // 用传统 read() 验证文件内容确实被改了
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

void Stage1Basic::demo_shared_vs_private() {
    printf("[Stage1] demo_shared_vs_private: SHARED vs PRIVATE (COW)\n");

    const std::string path_str = work_path("mmap_learn_cow.txt");
    const char *path = path_str.c_str();
    const char *original = "AAAA";
    size_t len = strlen(original);

    // lambda 表达式：在函数内部定义一个"局部小函数"，赋值给变量 write_file
    //   auto       : lambda 的类型由编译器生成、没有名字，只能用 auto 接收
    //   [&]        : 捕获列表，按引用使用外层变量（这里用到了 path 和 len）
    //   (content)  : 参数列表，和普通函数一样
    //   { ... }    : 函数体
    // 作用：用传统 I/O 把 content 写入文件，覆盖旧内容。
    // 下面 Part A、Part B 都要先把文件重置为 "AAAA"，封装成 lambda 避免重复代码。
    // 调用方式：write_file("AAAA");
    auto write_file = [&](const char *content) {
        FileDescriptor fd(path, O_CREAT | O_RDWR | O_TRUNC);
        fd.truncate(static_cast<off_t>(len));
        pwrite(fd.get(), content, len, 0);
    };

    // 同样是 lambda：用传统 read() 读取文件内容到 buf，并以 '\0' 结尾，方便 printf("%s")。
    // 用来验证"磁盘上的文件"到底有没有被 mmap 的修改改掉。
    //   bufsize - 1 : 给结尾的 '\0' 预留一个字节
    //   n <= 0      : 读取失败或文件为空，返回空字符串
    // 调用方式：char buf[16]; read_file(buf, sizeof(buf));
    auto read_file = [&](char *buf, size_t bufsize) {
        FileDescriptor fd(path, O_RDONLY);
        ssize_t n = ::read(fd.get(), buf, bufsize - 1);
        if (n > 0) buf[n] = '\0';
        else buf[0] = '\0';
    };

    // --- Part A：MAP_SHARED 的修改会写回文件 ---
    printf("  --- MAP_SHARED ---\n");
    write_file(original);
    //{}:块作用域,块结束，资源就释放，减少资源占用的时间
    {
        FileDescriptor fd(path, O_RDWR);

        MmapRegion region = MmapBuilder()
                                .length(len)
                                .readWrite()
                                .shared()
                                .file(fd.get())
                                .build();

        printf("  mmap content before: %.*s\n", static_cast<int>(len), region.as<const char>());

        // 改成 "BBBB"，并 msync 刷盘
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

    // --- Part B：MAP_PRIVATE 的修改不会写回文件（写时复制 COW）---
    printf("  --- MAP_PRIVATE (COW) ---\n");
    write_file(original);  // 重置为 "AAAA"
    {
        FileDescriptor fd(path, O_RDONLY);

        MmapRegion region = MmapBuilder()
                                .length(len)
                                .readWrite()
                                .priv()
                                .file(fd.get())
                                .build();

        printf("  mmap content before: %.*s\n", static_cast<int>(len), region.as<const char>());

        // 改成 "CCCC" —— 只改了本进程的私有副本
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

void Stage1Basic::run_all() {
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

} // namespace mmap_learn
