#include <iostream>
#include<sys/wait.h>/*引入wait函数的头文件*/
//#include<sys/reg.h>/* 对寄存器的常量值进行定义，如Eax，EBX....... */
#include<sys/proc_info.h>/* 对寄存器的常量值进行定义，如Eax，EBX....... */
#include<sys/user.h>/*gdb调试专用文件，里面有定义好的各种数据类型*/
#include<sys/ptrace.h>/*引入prtace头文件*/
#include<unistd.h>/*引入fork函数的头文件*/
#include<sys/syscall.h> /* SYS_write */



#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <mach/machine.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <csignal>

/***
 * rax: 累加器寄存器（Accumulator Register）
rbx: 基址寄存器（Base Register）
rcx: 计数寄存器（Count Register）
rdx: 数据寄存器（Data Register）
rdi: 目标寄存器（Destination Index Register）
rsi: 源寄存器（Source Index Register）
rbp: 基址指针寄存器（Base Pointer Register）
rsp: 栈指针寄存器（Stack Pointer Register）
r8 ~ r15: 通用寄存器（General Purpose Registers）
rip: 指令指针寄存器（Instruction Pointer Register）
rflags: 标志寄存器（Flags Register）
cs: 代码段寄存器（Code Segment Register）
fs: FS 段寄存器
gs: GS 段寄存器

 */
struct user_regs_struct {
    __uint64_t rax, rbx, rcx, rdx, rdi, rsi, rbp, rsp, r8, r9, r10, r11, r12, r13, r14, r15, rip, rflags, cs, fs, gs;
} regs;


int main() {
    pid_t child;
    child = fork();

    if (child == 0) {
        if (ptrace(PT_TRACE_ME, 0, nullptr, 0) == -1) {
            perror("ptrace");
            return 1;
        }
        raise(SIGSTOP);

        execl("/bin/ls", "ls", "-l", "-h", NULL);

    } else {
        // 等待子进程停止
        wait(nullptr);

        // 获取子进程的寄存器值
        auto reg_addr = (mach_vm_address_t) &regs;
        mach_vm_size_t reg_size = sizeof(regs);
        // 函数读取进程的内存数据
        kern_return_t result = mach_vm_read_overwrite(mach_task_self(), reg_addr, reg_size, (mach_vm_address_t) &regs,
                                                      &reg_size);
        if (result != KERN_SUCCESS) {
            fprintf(stderr, "Failed to read register values.\n");
            return 1;
        }

        // 打印寄存器的值
        printf("RIP: 0x%llx\n", regs.rip);
        printf("RAX: 0x%llx\n", regs.rax);

        // 继续子进程的执行
        if (ptrace(PT_CONTINUE, child, (caddr_t) 1, 0) == -1) {
            perror("ptrace");
            return 1;
        }

        // 等待子进程结束
        wait(nullptr);
        return 0;


    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
