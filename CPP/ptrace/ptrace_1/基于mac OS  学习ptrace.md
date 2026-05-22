# 基于 macOS 学习 ptrace

## 目录

1. [什么是 ptrace](#1-什么是-ptrace)
2. [macOS 与 Linux 的差异](#2-macos-与-linux-的差异)
3. [macOS ptrace 请求码速查](#3-macos-ptrace-请求码速查)
4. [环境准备与权限配置](#4-环境准备与权限配置)
5. [基础示例：PT_TRACE_ME](#5-基础示例pt_trace_me)
6. [进阶示例：读取子进程寄存器与内存](#6-进阶示例读取子进程寄存器与内存)
7. [ARM64 寄存器详解](#7-arm64-寄存器详解)
8. [Mach API 与 ptrace 的协作](#8-mach-api-与-ptrace-的协作)
9. [内存读写](#9-内存读写)
10. [单步执行与断点原理](#10-单步执行与断点原理)
11. [反调试：PT_DENY_ATTACH](#11-反调试pt_deny_attach)
12. [常见错误与排查](#12-常见错误与排查)
13. [完整工程示例](#13-完整工程示例)
14. [参考资料](#14-参考资料)

---

## 1. 什么是 ptrace

`ptrace`（process trace）是 Unix/POSIX 系统提供的一个系统调用，允许一个进程（**调试器/tracer**）观察并控制另一个进程（**被调试者/tracee**）的执行。

**调试器能做什么：**
- 读取/修改目标进程的内存
- 读取/修改目标进程的 CPU 寄存器
- 在目标进程执行特定指令时暂停它（断点）
- 单步执行目标进程
- 捕获目标进程发出的信号

`lldb`、`gdb`、`Instruments`、`strace`（Linux）等工具的底层都依赖 ptrace。

### 调用签名

```c
#include <sys/ptrace.h>

int ptrace(int request, pid_t pid, caddr_t addr, int data);
```

| 参数      | 说明                                         |
|-----------|----------------------------------------------|
| `request` | 操作码，例如 `PT_TRACE_ME`、`PT_CONTINUE`    |
| `pid`     | 目标进程 PID（`PT_TRACE_ME` 时填 0）         |
| `addr`    | 地址参数，含义随 request 变化               |
| `data`    | 数据参数，含义随 request 变化               |

返回值：成功返回 0，失败返回 -1 并设置 `errno`。

---

## 2. macOS 与 Linux 的差异

macOS 的 ptrace 是 **BSD ptrace 的子集**，功能比 Linux 少，但 macOS 额外提供了功能更强的 **Mach 内核接口**。

| 能力              | Linux ptrace | macOS ptrace | macOS Mach API |
|-------------------|:---:|:---:|:---:|
| 附加到进程         | YES | YES | YES |
| 读取寄存器         | YES | NO  | YES |
| 写入寄存器         | YES | NO  | YES |
| 读取内存（逐字）   | YES (PEEKDATA) | NO  | YES |
| 写入内存（逐字）   | YES (POKEDATA) | NO  | YES |
| 单步执行           | YES | YES (PT_STEP) | -- |
| 系统调用跟踪       | YES (PTRACE_SYSCALL) | NO  | -- |
| 反调试检测         | NO  | YES (PT_DENY_ATTACH) | -- |

**结论：** 在 macOS 上，`ptrace` 主要用于**附加控制**（允许被调试、继续执行、单步等），读写寄存器和内存需要配合 **Mach API**（`task_for_pid`、`thread_get_state`、`mach_vm_read_overwrite` 等）。

---

## 3. macOS ptrace 请求码速查

```c
// 定义在 <sys/ptrace.h>
PT_TRACE_ME     // 子进程声明允许父进程跟踪自己
PT_READ_I       // 读取指令内存（字）
PT_READ_D       // 读取数据内存（字）
PT_READ_U       // 读取 user 区域
PT_WRITE_I      // 写入指令内存（字）
PT_WRITE_D      // 写入数据内存（字）
PT_WRITE_U      // 写入 user 区域
PT_CONTINUE     // 恢复执行；data 为发送的信号（0 = 不发）
PT_KILL         // 发送 SIGKILL 终止目标进程
PT_STEP         // 单步执行一条指令
PT_ATTACH       // 附加到一个已运行的进程
PT_DETACH       // 解除附加，恢复目标进程
PT_DENY_ATTACH  // 拒绝任何调试器附加（反调试）
```

> **注意：** macOS 没有 `PTRACE_PEEKDATA` / `PTRACE_POKEDATA`（Linux 专有），
> 也没有 `PTRACE_GETREGS` / `PTRACE_SETREGS`，这些功能由 Mach API 替代。

---

## 4. 环境准备与权限配置

### 4.1 SIP（系统完整性保护）

macOS 的 SIP 限制了 `task_for_pid` 的使用——普通进程无法获取其他进程的 Mach task 句柄。

**开发调试时的解决方案（三选一）：**

**方案 A：父子进程关系（推荐学习使用）**

`fork` 出的子进程天然允许父进程调用 `task_for_pid`，无需关闭 SIP。本教程所有示例均采用此方案。

**方案 B：赋予可执行文件调试权限（entitlement）**

创建 `entitlements.plist`：
```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>com.apple.security.cs.debugger</key>
  <true/>
</dict>
</plist>
```

签名并赋权：
```bash
codesign -s - --entitlements entitlements.plist -f ./my_debugger
```

**方案 C：关闭 SIP（仅用于开发机，有安全风险）**

重启进入恢复模式（Apple Silicon：长按电源键），在终端执行：
```bash
csrutil disable
```

### 4.2 CMakeLists.txt 模板

```cmake
cmake_minimum_required(VERSION 3.20)
project(ptrace_demo)

set(CMAKE_CXX_STANDARD 17)

add_executable(ptrace_demo main.cpp)

# 链接 Mach 框架
target_link_libraries(ptrace_demo "-framework CoreFoundation")
```

> macOS 上 Mach API 不需要显式链接，头文件位于
> `/usr/include/mach/` 和 `/usr/include/sys/ptrace.h`。

---

## 5. 基础示例：PT_TRACE_ME

这是 ptrace 最简单的用法：子进程向内核声明"我允许被我的父进程调试"。

```cpp
// ptrace_1/main.cpp
#include <cstdio>
#include <sys/ptrace.h>

int main() {
    int a = 10;

    // PT_TRACE_ME：告诉内核允许父进程跟踪本进程
    // 返回 -1 说明已经有调试器附加（或调用失败）
    if (ptrace(PT_TRACE_ME, 0, 0, 0) == -1) {
        printf("already being traced!\n");
        a = 30;
    }

    a = 45;
    printf("no one traces me!\n");
    return 0;
}
```

### 运行逻辑

```
直接运行（无调试器）：
  ptrace(PT_TRACE_ME) 成功 → a = 45 → 打印 "no one traces me!"

用 lldb 运行：
  ptrace(PT_TRACE_ME) 返回 -1 → a = 30 → a = 45 → 打印 "already being traced!"
```

### 关键点

- `PT_TRACE_ME` 只能由**子进程自身**调用，pid 参数填 0。
- 每个进程只能被一个 tracer 跟踪（内核维护一个 `p_pptr` 字段）。
- 调用后，进程在收到任何信号时会自动暂停并通知父进程（通过 `wait()`）。

---

## 6. 进阶示例：读取子进程寄存器与内存

完整代码见 `ptrace_0/main.cpp`，下面逐段讲解。

### 6.1 整体流程

```
main()
  │
  ├─ fork()
  │
  ├─ [子进程]
  │    ├─ write_msg()           // 向全局变量写 "sw"，地址存入 x0
  │    ├─ ptrace(PT_TRACE_ME)   // 声明允许被父进程调试
  │    ├─ raise(SIGSTOP)        // 主动暂停，通知父进程
  │    └─ execl("ls -l -h")    // 恢复后执行
  │
  └─ [父进程]
       ├─ wait()                // 等子进程因 SIGSTOP 暂停
       ├─ task_for_pid()        // 获取子进程 Mach task 句柄
       ├─ task_threads()        // 获取子进程线程列表
       ├─ thread_get_state()    // 读取 ARM64 寄存器
       ├─ 打印 PC/SP/FP/x0
       ├─ mach_vm_read_overwrite() // 从子进程内存读数据
       ├─ ptrace(PT_CONTINUE)   // 放行子进程
       └─ wait()                // 等子进程结束
```

### 6.2 子进程：声明可跟踪并暂停

```cpp
if (child == 0) {
    write_msg();  // 写入 "sw" 到全局变量，x0 寄存器保存该地址

    if (ptrace(PT_TRACE_ME, 0, nullptr, 0) == -1) {
        perror("ptrace PT_TRACE_ME");
        return 1;
    }

    raise(SIGSTOP);  // 向自己发送 SIGSTOP，父进程的 wait() 会返回
    execl("/bin/ls", "ls", "-l", "-h", nullptr);
}
```

**为什么用 `raise(SIGSTOP)` 而不是直接 `sleep`？**

`SIGSTOP` 是一个专门的暂停信号，内核会将进程状态设为 `TSTOP`，父进程的 `wait()` 能感知到这个状态变化，从而精确知道子进程已暂停。`sleep` 只是延时，无法保证父进程在正确时机读取寄存器。

### 6.3 父进程：获取子进程 Mach task

```cpp
// 等待子进程暂停
wait(nullptr);

task_t child_task;
kern_return_t kr = task_for_pid(mach_task_self(), child, &child_task);
if (kr != KERN_SUCCESS) {
    fprintf(stderr, "task_for_pid failed: %s\n", mach_error_string(kr));
    return 1;
}
```

`task_for_pid` 是 macOS 的核心调试接口，返回目标进程的 **Mach task port**，后续所有内存/寄存器操作都通过这个句柄进行。

---

## 7. ARM64 寄存器详解

### 7.1 通用寄存器

| 寄存器     | 别名  | 用途                            | x86-64 对应 |
|-----------|-------|---------------------------------|-------------|
| `x0`~`x7` | --    | 函数参数（前 8 个）/ 返回值     | rdi,rsi...  |
| `x8`      | --    | 间接结果寄存器（大结构体返回）   | --          |
| `x9`~`x15`| --    | 调用者保存的临时寄存器           | r10,r11...  |
| `x16`~`x17`| ip0,ip1 | 过程链接临时寄存器（PLT/trampoline） | -- |
| `x18`     | --    | 平台保留（macOS 用于 TLS）       | --          |
| `x19`~`x28`| --   | 被调用者保存的寄存器             | rbx,r12...  |
| `x29`     | `fp`  | 帧指针（Frame Pointer）          | rbp         |
| `x30`     | `lr`  | 链接寄存器（返回地址）           | --（x86 用栈）|
| `sp`      | --    | 栈指针                          | rsp         |
| `pc`      | --    | 程序计数器（当前指令地址）       | rip         |
| `cpsr`    | --    | 当前程序状态寄存器（标志位）     | rflags      |

### 7.2 读取寄存器（thread_get_state）

```cpp
arm_thread_state64_t state;
mach_msg_type_number_t count = ARM_THREAD_STATE64_COUNT;

kern_return_t kr = thread_get_state(
    threads[0],              // 线程句柄
    ARM_THREAD_STATE64,      // 寄存器集类型
    (thread_state_t)&state,  // 输出缓冲区
    &count                   // 输入/输出：缓冲区大小
);
```

### 7.3 Apple Silicon PAC（指针认证）注意事项

Apple Silicon（M1/M2/M3）启用了 **PAC（Pointer Authentication Codes）**，`pc`、`sp`、`fp` 字段的高位嵌入了认证码，直接读取会得到错误的地址。

**必须使用专用宏：**

```cpp
// 正确：使用宏剥离 PAC 位
uint64_t pc = (uint64_t)arm_thread_state64_get_pc(state);
uint64_t sp = (uint64_t)arm_thread_state64_get_sp(state);
uint64_t fp = (uint64_t)arm_thread_state64_get_fp(state);

// 错误：直接访问字段（高位含 PAC，地址不正确）
// uint64_t pc = state.__pc;  // 不要这样做
```

通用寄存器 `x0`~`x28` 不受 PAC 影响，可以直接读取：
```cpp
printf("x0 = 0x%llx\n", state.__x[0]);
printf("x1 = 0x%llx\n", state.__x[1]);
// ...
printf("lr = 0x%llx\n", state.__lr);  // x30
```

---

## 8. Mach API 与 ptrace 的协作

macOS 调试的典型模式是：**ptrace 负责控制流，Mach API 负责数据读写**。

```
ptrace 的职责：
  PT_TRACE_ME   → 建立父子调试关系
  PT_ATTACH     → 附加到已有进程
  PT_CONTINUE   → 恢复执行（可带信号）
  PT_STEP       → 单步执行一条指令
  PT_DETACH     → 解除附加
  PT_KILL       → 终止目标进程

Mach API 的职责：
  task_for_pid()           → 获取目标进程句柄
  task_threads()           → 枚举线程
  thread_get_state()       → 读取寄存器
  thread_set_state()       → 修改寄存器
  mach_vm_read_overwrite() → 读取进程内存
  mach_vm_write()          → 写入进程内存
  mach_vm_protect()        → 修改内存保护属性
```

### 常用 Mach API 签名

```cpp
#include <mach/mach.h>
#include <mach/mach_vm.h>

// 获取进程 task
kern_return_t task_for_pid(mach_port_t host, pid_t pid, task_t *task);

// 获取线程列表
kern_return_t task_threads(
    task_t target_task,
    thread_act_array_t *act_list,
    mach_msg_type_number_t *act_listCnt
);

// 读取线程寄存器状态
kern_return_t thread_get_state(
    thread_act_t target_act,
    thread_state_flavor_t flavor,    // ARM_THREAD_STATE64
    thread_state_t old_state,        // 输出：arm_thread_state64_t*
    mach_msg_type_number_t *old_stateCnt
);

// 写入线程寄存器状态
kern_return_t thread_set_state(
    thread_act_t target_act,
    thread_state_flavor_t flavor,
    thread_state_t new_state,
    mach_msg_type_number_t new_stateCnt
);

// 读取进程虚拟内存
kern_return_t mach_vm_read_overwrite(
    vm_map_t target_task,
    mach_vm_address_t address,    // 目标进程中的地址
    mach_vm_size_t size,          // 读取字节数
    mach_vm_address_t data,       // 本地缓冲区地址
    mach_vm_size_t *outsize       // 实际读取字节数
);

// 写入进程虚拟内存
kern_return_t mach_vm_write(
    vm_map_t target_task,
    mach_vm_address_t address,    // 目标进程中的地址
    vm_offset_t data,             // 本地数据地址
    mach_msg_type_number_t dataCnt
);
```

---

## 9. 内存读写

### 9.1 读取子进程内存

```cpp
char buf[64] = {0};
mach_vm_size_t out_size = 0;

kern_return_t kr = mach_vm_read_overwrite(
    child_task,
    (mach_vm_address_t)target_address,  // 目标进程中的地址
    sizeof(buf),
    (mach_vm_address_t)buf,             // 本地缓冲区
    &out_size
);

if (kr == KERN_SUCCESS) {
    printf("读到 %llu 字节: %s\n", out_size, buf);
} else {
    fprintf(stderr, "读取失败: %s\n", mach_error_string(kr));
}
```

### 9.2 写入子进程内存

写入前需要先确保目标内存有写权限：

```cpp
// 修改内存保护，允许写入
mach_vm_protect(child_task, target_address, size,
                FALSE, VM_PROT_READ | VM_PROT_WRITE);

// 写入数据
const char new_data[] = "hello";
kern_return_t kr = mach_vm_write(
    child_task,
    target_address,
    (vm_offset_t)new_data,
    sizeof(new_data)
);

// 恢复原来的保护属性（防止写入了代码段后崩溃）
mach_vm_protect(child_task, target_address, size,
                FALSE, VM_PROT_READ | VM_PROT_EXECUTE);
```

### 9.3 fork 后的地址空间

`fork` 后，父子进程的**虚拟地址布局完全相同**，全局变量、函数的地址在父子进程中一致（但物理页通过 COW 隔离）。这意味着：

```cpp
// 父进程中可以直接用自己看到的全局变量地址来读取子进程的数据
volatile char g_msg[8] = {0};

// 子进程写入了 g_msg
// 父进程用同一个地址读取子进程的 g_msg
kr = mach_vm_read_overwrite(child_task,
    (mach_vm_address_t)g_msg,  // 父进程的 g_msg 地址 == 子进程的 g_msg 地址
    sizeof(buf),
    (mach_vm_address_t)buf,
    &out_size);
```

---

## 10. 单步执行与断点原理

### 10.1 单步执行（PT_STEP）

`PT_STEP` 让目标进程执行**恰好一条指令**后暂停，父进程可在每条指令执行后检查寄存器状态。

```cpp
// 单步执行，不发送额外信号（data = 0）
if (ptrace(PT_STEP, child_pid, (caddr_t)1, 0) == -1) {
    perror("ptrace PT_STEP");
}

// 等待子进程再次暂停
int status;
waitpid(child_pid, &status, 0);

if (WIFSTOPPED(status)) {
    // 再次读取寄存器，观察单步后的变化
    arm_thread_state64_t state;
    // ... thread_get_state() ...
    printf("单步后 PC = 0x%llx\n",
           (uint64_t)arm_thread_state64_get_pc(state));
}
```

### 10.2 软件断点原理（x86 vs ARM64）

**x86/x64：**
- 断点指令：`INT 3`（机器码 `0xCC`，1 字节）
- 在目标地址写入 `0xCC`，CPU 执行时触发 `SIGTRAP`
- 恢复时：还原原始字节，将 RIP 回退 1 字节，继续执行

**ARM64：**
- 断点指令：`BRK #0`（机器码 `0xD4200000`，4 字节，固定长度）
- 在目标地址写入 `0xD4200000`，CPU 执行时触发 `SIGTRAP`
- 恢复时：还原原始 4 字节，PC 无需回退（ARM64 异常不会自动推进 PC）

```cpp
// ARM64 软件断点实现思路
const uint32_t BRK_INST = 0xD4200000;  // BRK #0
uint32_t original_inst;

// 1. 读取原始指令
mach_vm_read_overwrite(task, bp_addr, 4,
    (mach_vm_address_t)&original_inst, &out_size);

// 2. 写入断点指令
mach_vm_protect(task, bp_addr, 4, FALSE, VM_PROT_READ | VM_PROT_WRITE);
mach_vm_write(task, bp_addr, (vm_offset_t)&BRK_INST, 4);
mach_vm_protect(task, bp_addr, 4, FALSE, VM_PROT_READ | VM_PROT_EXECUTE);

// 3. 等待 SIGTRAP
waitpid(child_pid, &status, 0);
// WSTOPSIG(status) == SIGTRAP

// 4. 恢复原始指令，继续执行
mach_vm_protect(task, bp_addr, 4, FALSE, VM_PROT_READ | VM_PROT_WRITE);
mach_vm_write(task, bp_addr, (vm_offset_t)&original_inst, 4);
mach_vm_protect(task, bp_addr, 4, FALSE, VM_PROT_READ | VM_PROT_EXECUTE);
ptrace(PT_CONTINUE, child_pid, (caddr_t)1, 0);
```

### 10.3 waitpid 状态解析

```cpp
int status;
pid_t result = waitpid(child_pid, &status, 0);

if (WIFEXITED(status)) {
    printf("进程正常退出，退出码: %d\n", WEXITSTATUS(status));
} else if (WIFSTOPPED(status)) {
    int sig = WSTOPSIG(status);
    printf("进程被信号 %d 暂停\n", sig);
    // sig == SIGTRAP  → 断点或单步
    // sig == SIGSTOP  → raise(SIGSTOP)
} else if (WIFSIGNALED(status)) {
    printf("进程被信号 %d 终止\n", WTERMSIG(status));
}
```

---

## 11. 反调试：PT_DENY_ATTACH

`PT_DENY_ATTACH` 是 macOS 独有的 ptrace 请求，调用后当前进程拒绝任何调试器附加。常见于商业应用的反调试保护。

```cpp
// 进程启动时调用，阻止调试器附加
if (ptrace(PT_DENY_ATTACH, 0, 0, 0) == -1) {
    // 如果已经有调试器在附加，进程会收到 SIGKILL 并终止
    // 普通调用不会到这里
}
```

**行为：**
- 调用后，如果有调试器尝试 `PT_ATTACH`，内核拒绝并返回 `ENOTSUP`
- 如果调用时**已经有调试器附加**，进程会立即收到 `SIGKILL`

**绕过方式（学习目的）：**
1. 在调用 `PT_DENY_ATTACH` 前用 lldb 在该调用处设断点，跳过这行代码
2. 修改二进制，将 `PT_DENY_ATTACH` 的请求码（`31`）替换为无效值

```bash
# 用 lldb 绕过 PT_DENY_ATTACH 的示例
$ lldb ./target_app
(lldb) breakpoint set --name ptrace
(lldb) run
# 在 ptrace 处停下，检查参数
(lldb) register read x0   # x0 == 31 就是 PT_DENY_ATTACH
(lldb) register write x0 0  # 改成 PT_TRACE_ME(0) 让 ptrace 无效
(lldb) continue
```

---

## 12. 常见错误与排查

### KERN_FAILURE / task_for_pid 返回错误

```
task_for_pid failed: (os/kern) failure
```

**原因：** SIP 限制，或目标进程不是调用者的子进程。

**排查：**
1. 确认是 `fork()` 出的子进程，父子进程关系下无需特殊权限
2. 若需要调试任意进程，添加 `com.apple.security.cs.debugger` entitlement
3. 最后手段：关闭 SIP

### ptrace 返回 EPERM

```
ptrace: Operation not permitted
```

**原因：**
- 目标进程调用了 `PT_DENY_ATTACH`
- 尝试跟踪 root 进程而当前用户非 root
- 目标进程已被其他调试器跟踪

### mach_vm_read_overwrite 返回 KERN_INVALID_ADDRESS

```
mach_vm_read_overwrite failed: (os/kern) invalid address
```

**原因：** 地址无效（可能是空指针、已释放内存、或未映射区域）。

**排查：** 使用 `vmmap <pid>` 查看目标进程的内存映射：
```bash
vmmap <pid>
```

### 内存写入失败

写入前必须先用 `mach_vm_protect` 开放写权限，代码段默认为 `r-x`，不可直接写入。

---

## 13. 完整工程示例

### ptrace_0 — 原始 Mach API 直接调用

`ptrace_0/main.cpp` 用最少的代码演示核心流程：
`fork` → `PT_TRACE_ME` + `raise(SIGSTOP)` → `task_for_pid` → `thread_get_state` → `mach_vm_read_overwrite` → `PT_CONTINUE`

```bash
cd ptrace_0 && cmake -B build && cmake --build build && ./build/ptrace_0
```

---

### ptrace_1 — 设计模式重构的完整 Demo 套件

`ptrace_1/` 用五种设计模式将所有知识点整合为一套可运行的演示程序。

**文件结构：**
```
ptrace_1/
├── CMakeLists.txt          # file(WRITE) 内联生成 entitlements，无需外部文件
├── main.cpp                # Demo 入口（Template Method 多态调用）
├── core/
│   ├── MachTask.h          # RAII：自动管理 Mach port 生命周期
│   ├── RegisterAccess.h/cpp  # ARM64 寄存器读写（PAC 安全宏）
│   ├── MemoryAccess.h/cpp    # 内存读写（data/code 两条路径）
│   ├── ProcessCtrl.h/cpp     # ptrace 控制原语封装
│   ├── Breakpoint.h/cpp      # Command 模式：断点 enable/disable
│   └── Debugger.h/cpp        # Facade + Factory Method
└── demos/
    ├── IDemo.h               # Template Method 接口
    ├── DemoTraceMe           # Demo 1: PT_TRACE_ME 调试器检测
    ├── DemoRegisters         # Demo 2: 读取 ARM64 寄存器
    ├── DemoMemory            # Demo 3: 读写子进程内存
    ├── DemoSingleStep        # Demo 4: PT_STEP 单步执行
    └── DemoAntiDebug         # Demo 5: PT_DENY_ATTACH 反调试
```

**构建运行：**
```bash
cd ptrace_1
cmake -B build && cmake --build build
./build/ptrace_demos
```

**实际运行输出（macOS Apple Silicon）：**

```
Demo 1: PT_TRACE_ME
  [child] PT_TRACE_ME 返回 0，未检测到调试器
  [parent] 子进程因 SIGSTOP 暂停，正在放行...
  [child] 被父进程放行，继续运行
  提示：用 lldb 运行本程序可触发"检测到调试器"分支

Demo 2: ReadRegisters
  子进程 PID = 13710，已冻结，读取寄存器：
  PC  = 0x00000001912aa388  ← raise() 系统调用返回后的下一条指令
  SP  = 0x000000016bd7aa20  ← 当前栈顶（0x16x... 是栈区特征）
  FP  = 0x000000016bd7aa40
  LR  = 0x00000001912e388c  ← raise() 的调用者返回地址（libc 内部）
  x0~x7 = 各寄存器当前值

Demo 3: ReadWriteMemory
  [parent] 读取子进程 g_buf = "hello, ptrace!"
  [parent] 写入子进程 g_buf = "PATCHED!"
  [parent] 验证读回   g_buf = "PATCHED!"
  [child]  child_fn 读到 g_buf = "PATCHED!"   ← 内存修改跨进程生效

Demo 4: SingleStep
  初始 PC = 0x00000001912aa388
  step  1: PC = 0x00000001912aa3a8  ← 跳转（raise 返回路径）
  step  2: PC = 0x00000001912e388c  ← 进入 libc raise() 返回栈帧
  step  3~7: PC 按 +4 递增             ← ARM64 定长指令顺序执行
  step  8~12: PC 跳入另一地址段        ← 进入下一层调用（signal 处理）

Demo 5: AntiDebug
  PT_DENY_ATTACH 成功，P_LNOATTACH 已设置
  PT_TRACE_ME 返回 0，无调试器
```

**关于 Demo 4 单步输出的解读：**

子进程停在 `raise(SIGSTOP)` 返回处（PC = `0x...912aa388`）。前几步在 `raise()` → `libc` 的返回路径中，第 3～7 步 PC 按 +4 递增（ARM64 定长指令），第 8 步跳到新地址段是进入 `_sigtramp` 或 dyld 的信号恢复路径。最终子进程才会进入 `child_fn` 的代码，需要更多步才到达我们写的 `volatile` 赋值指令。

---

## 14. 参考资料

- `man 2 ptrace` — macOS ptrace 手册页
- `man 2 wait` — waitpid / wait 状态解析
- [XNU 内核源码（bsd/kern/mach_process.c）](https://github.com/apple-oss-distributions/xnu)
- Apple Developer Documentation: [Mach Kernel Interface](https://developer.apple.com/documentation/kernel/mach)
- [linux-沙盒入门，ptrace 从 0 到 1](https://cloud.tencent.com/developer/article/1799705)
- [Writing a Linux Debugger](https://blog.tartanllama.xyz/writing-a-linux-debugger-setup/) — 原理相通，Linux 视角
- `<mach/arm/thread_status.h>` — ARM64 寄存器结构体定义
- `<mach/mach_vm.h>` — 虚拟内存操作 API