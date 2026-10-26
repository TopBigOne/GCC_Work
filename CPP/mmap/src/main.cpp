#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "stage1/stage1_basic.h"
#include "stage2/stage2_syscalls.h"
#include "stage3/stage3_anonymous.h"
#include "stage4/stage4_shared_mem.h"
#include "stage5/stage5_device.h"
#include "stage7/stage7_kernel.h"
#include "stage8/stage8_benchmark.h"
#include "stage9/stage9_projects.h"

// 准备临时文件目录：在当前工作目录下创建 mmap_tmp/（kWorkDir），并清空上次运行留下的文件。
// 这个目录只有本程序使用，所以可以放心清空；同时清掉 stage4 可能残留的 POSIX 共享内存对象
static bool prepare_work_dir() {
    using mmap_learn::kWorkDir;

    // 目录不存在就创建；已存在（EEXIST）也算成功
    if (mkdir(kWorkDir, 0755) != 0 && errno != EEXIST) {
        printf("[ERROR] 无法创建临时目录 %s: %s\n", kWorkDir, strerror(errno));
        return false;
    }
    // 同名的如果是普通文件而不是目录，后面的 demo 都会失败，提前报错
    struct stat dir_st {};
    if (stat(kWorkDir, &dir_st) != 0 || !S_ISDIR(dir_st.st_mode)) {
        printf("[ERROR] %s 已存在但不是目录\n", kWorkDir);
        return false;
    }

    int removed = 0;
    if (DIR *dir = opendir(kWorkDir)) {
        while (dirent *entry = readdir(dir)) {
            // 只删普通文件；"."、".." 和子目录等其他类型跳过
            std::string path = std::string(kWorkDir) + "/" + entry->d_name;
            struct stat st {};
            if (lstat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode) && unlink(path.c_str()) == 0) {
                ++removed;
            }
        }
        closedir(dir);
    }

    // 程序崩溃时 stage4 来不及 shm_unlink，共享内存对象会一直留在系统里
    static const char *kShmNames[] = {"/mmap_learn_stage4", "/mmap_learn_ring"};
    for (const char *name : kShmNames) {
        if (shm_unlink(name) == 0) ++removed;
    }

    // 打印绝对路径，方便到文件管理器或终端里查看生成的文件
    char abs[PATH_MAX];
    printf("[workdir] 临时文件目录: %s", realpath(kWorkDir, abs) ? abs : kWorkDir);
    if (removed > 0) printf("（已清理上次运行残留的 %d 项）", removed);
    printf("\n");
    return true;
}

static void print_menu() {
    printf("\n");
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║         Linux mmap 学习 - 阶段选择          ║\n");
    printf("╠══════════════════════════════════════════════╣\n");
    printf("║  1. 基本用法 (read/write/shared vs private) ║\n");
    printf("║  2. 配套系统调用 (msync/mprotect/madvise)   ║\n");
    printf("║  3. 匿名映射与 Arena 内存池                 ║\n");
    printf("║  4. 进程间共享内存 (Producer/Consumer)      ║\n");
    printf("║  5. 设备与驱动层 mmap                       ║\n");
    printf("║  7. 内核原理观测                            ║\n");
    printf("║  8. 性能基准测试 (mmap vs read/write)       ║\n");
    printf("║  9. 综合实战 (MiniGrep / KVStore)           ║\n");
    printf("║  0. 运行全部阶段                            ║\n");
    printf("║  q. 退出                                    ║\n");
    printf("╚══════════════════════════════════════════════╝\n");
    printf("请选择阶段: ");
}

int main(int argc, char *argv[]) {
    if (!prepare_work_dir()) return 1;

    // 支持命令行直接指定阶段: ./mmap 1
    if (argc > 1) {
        int stage = atoi(argv[1]);
        switch (stage) {
            case 1: mmap_learn::Stage1Basic::run_all(); break;
            case 2: mmap_learn::Stage2Syscalls::run_all(); break;
            case 3: mmap_learn::Stage3Anonymous::run_all(); break;
            case 4: mmap_learn::Stage4SharedMem::run_all(); break;
            case 5: mmap_learn::Stage5Device::run_all(); break;
            case 7: mmap_learn::Stage7Kernel::run_all(); break;
            case 8: mmap_learn::Stage8Benchmark::run_all(); break;
            case 9: mmap_learn::Stage9Projects::run_all(); break;
            case 0:
                mmap_learn::Stage1Basic::run_all();
                mmap_learn::Stage2Syscalls::run_all();
                mmap_learn::Stage3Anonymous::run_all();
                mmap_learn::Stage4SharedMem::run_all();
                mmap_learn::Stage5Device::run_all();
                mmap_learn::Stage7Kernel::run_all();
                mmap_learn::Stage8Benchmark::run_all();
                mmap_learn::Stage9Projects::run_all();
                break;
            default: printf("未知阶段: %d\n", stage); return 1;
        }
        return 0;
    }

    // 交互式菜单
    while (true) {
        print_menu();

        char choice;
        if (scanf(" %c", &choice) != 1) break;

        switch (choice) {
            case '1': mmap_learn::Stage1Basic::run_all(); break;
            case '2': mmap_learn::Stage2Syscalls::run_all(); break;
            case '3': mmap_learn::Stage3Anonymous::run_all(); break;
            case '4': mmap_learn::Stage4SharedMem::run_all(); break;
            case '5': mmap_learn::Stage5Device::run_all(); break;
            case '7': mmap_learn::Stage7Kernel::run_all(); break;
            case '8': mmap_learn::Stage8Benchmark::run_all(); break;
            case '9': mmap_learn::Stage9Projects::run_all(); break;
            case '0':
                mmap_learn::Stage1Basic::run_all();
                mmap_learn::Stage2Syscalls::run_all();
                mmap_learn::Stage3Anonymous::run_all();
                mmap_learn::Stage4SharedMem::run_all();
                mmap_learn::Stage5Device::run_all();
                mmap_learn::Stage7Kernel::run_all();
                mmap_learn::Stage8Benchmark::run_all();
                mmap_learn::Stage9Projects::run_all();
                break;
            case 'q':
            case 'Q': printf("再见!\n"); return 0;
            default: printf("无效选择: %c\n", choice); break;
        }
    }

    return 0;
}
