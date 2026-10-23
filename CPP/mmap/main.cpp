#include <cstdio>
#include <cstdlib>

#include "src/stage1_basic.hpp"
#include "src/stage2_syscalls.hpp"
#include "src/stage3_anonymous.hpp"
#include "src/stage4_shared_mem.hpp"
#include "src/stage5_device.hpp"
#include "src/stage7_kernel.hpp"
#include "src/stage8_benchmark.hpp"
#include "src/stage9_projects.hpp"

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
