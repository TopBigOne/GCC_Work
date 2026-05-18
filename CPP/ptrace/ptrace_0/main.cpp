#include <csignal>
#include <iostream>
#include <mach/arm/thread_status.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// fork 后父子进程虚拟地址空间布局相同，全局变量地址一致
volatile char g_msg[8] = {0};

// 将 "sw" 写入全局变量，返回其地址（返回值自动存入 x0 寄存器）
const char* write_msg() {
  memcpy((void*)g_msg, "sw", 3);
  return (const char*)g_msg;
}

/***
 * macOS ARM64 寄存器（通过 arm_thread_state64_t 访问）
 * x0 ~ x7  : 函数参数 / 返回值寄存器
 * x8       : 间接结果寄存器
 * x9 ~ x15 : 调用者保存的临时寄存器
 * x16 ~ x17: 过程链接临时寄存器（PLT 跳转用）
 * x18      : 平台保留寄存器
 * x19 ~ x28: 被调用者保存的寄存器
 * x29 (fp) : 帧指针（Frame Pointer），等价于 x86 的 RBP
 * x30 (lr) : 链接寄存器（Link Register），保存返回地址
 * sp       : 栈指针（Stack Pointer），等价于 x86 的 RSP
 * pc       : 程序计数器（Program Counter），等价于 x86 的 RIP
 * cpsr     : 当前程序状态寄存器（Flags），等价于 x86 的 RFLAGS
 *
 * 注意：Apple Silicon 启用了指针认证（PAC），访问 pc/sp/fp
 *       需使用宏 arm_thread_state64_get_pc/sp/fp() 而非直接读取字段
 */

int main() {
  pid_t child = fork();

  if (child == 0) {
    // 子进程：写入字符串 "sw"，地址留在 x0 寄存器
    write_msg();

    // 允许父进程跟踪自己
    if (ptrace(PT_TRACE_ME, 0, nullptr, 0) == -1) {
      perror("ptrace");
      return 1;
    }
    raise(SIGSTOP); // 暂停，等待父进程

    execl("/bin/ls", "ls", "-l", "-h", NULL);

  } else {
    // 父进程：等待子进程停止
    wait(nullptr);

    // 通过 task_for_pid 获取子进程的 Mach task
    task_t child_task;
    kern_return_t kr = task_for_pid(mach_task_self(), child, &child_task);
    if (kr != KERN_SUCCESS) {
      fprintf(stderr, "task_for_pid failed: %s\n", mach_error_string(kr));
      fprintf(
          stderr,
          "提示：macOS 需要关闭 SIP 或赋予调试权限才能访问其他进程的 task\n");
      return 1;
    }

    // 获取子进程的线程列表
    thread_act_array_t threads;
    mach_msg_type_number_t thread_count;
    kr = task_threads(child_task, &threads, &thread_count);
    if (kr != KERN_SUCCESS) {
      fprintf(stderr, "task_threads failed: %s\n", mach_error_string(kr));
      return 1;
    }

    // 读取第一个线程的 ARM64 寄存器状态
    arm_thread_state64_t state;
    mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;
    kr = thread_get_state(threads[0], ARM_THREAD_STATE64,
                          (thread_state_t)&state, &count);
    if (kr != KERN_SUCCESS) {
      fprintf(stderr, "thread_get_state failed: %s\n", mach_error_string(kr));
      return 1;
    }

    // 打印寄存器的值（ARM64）
    // PC：程序计数器，等价于 x86 的 RIP
    printf("PC : 0x%llx\n", (uint64_t)arm_thread_state64_get_pc(state));
    // SP：栈指针，等价于 x86 的 RSP
    printf("SP : 0x%llx\n", (uint64_t)arm_thread_state64_get_sp(state));
    // FP：帧指针（x29），等价于 x86 的 RBP
    printf("FP : 0x%llx\n", (uint64_t)arm_thread_state64_get_fp(state));
    // x0：第一个参数/返回值寄存器，等价于 x86 的 RAX（此处存放 g_msg 地址）
    printf("x0 : 0x%llx\n", state.__x[0]);

    // 用 x0 中的地址从子进程内存读取字符串
    char buf[8] = {0};
    mach_vm_size_t out_size = 0;
    kr = mach_vm_read_overwrite(child_task,
                                (mach_vm_address_t)g_msg, // fork 后地址与子进程相同
                                sizeof(buf),
                                (mach_vm_address_t)buf,
                                &out_size);
    if (kr != KERN_SUCCESS) {
      fprintf(stderr, "mach_vm_read_overwrite failed: %s\n", mach_error_string(kr));
    } else {
      printf("从子进程内存读到: %s\n", buf);
    }

    // 释放线程列表内存
    vm_deallocate(mach_task_self(), (vm_address_t)threads,
                  thread_count * sizeof(thread_act_t));

    // 继续子进程的执行
    if (ptrace(PT_CONTINUE, child, (caddr_t)1, 0) == -1) {
      perror("ptrace");
      return 1;
    }

    // 等待子进程结束
    wait(nullptr);
    return 0;
  }

  return 0;
}
