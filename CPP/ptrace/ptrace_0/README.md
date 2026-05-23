# ptrace_0 - macOS ARM64 进程跟踪入门

## 简介

本项目是 `ptrace` + Mach API 的入门学习 Demo，运行在 **macOS Apple Silicon（ARM64）** 平台。

演示了如何通过 `ptrace` 和 macOS Mach 内核接口，在父进程中读取子进程的 CPU 寄存器状态。

## 执行流程

代码做了一件事：**父进程暂停子进程，读取它的 CPU 寄存器，然后放行。**

```
main()
  │
  ├─ fork()
  │
  ├─ [子进程]
  │    ├─ ptrace(PT_TRACE_ME)   // 声明"我允许被调试"
  │    ├─ raise(SIGSTOP)        // 主动暂停，等父进程来检查
  │    └─ execl("ls -l -h")    // 恢复后执行这个命令（占位目标程序，验证 PT_CONTINUE 生效）
  │
  └─ [父进程]
       ├─ wait()                 // 等子进程停下来
       ├─ task_for_pid()         // 拿到子进程的 Mach task 句柄
       ├─ task_threads()         // 获取子进程的线程列表
       ├─ thread_get_state()     // 读取第一个线程的寄存器
       ├─ 打印 PC / SP / FP / x0 // 输出寄存器当前值
       ├─ ptrace(PT_CONTINUE)    // 放行子进程
       └─ wait()                 // 等子进程结束
```

子进程冻结期间，父进程读到的就是子进程当前真实的寄存器值：

```
子进程（被调试者）          父进程（调试器）
      │
      │  ptrace(PT_TRACE_ME)
      │  raise(SIGSTOP)  ──────────────→  wait() 收到信号，子进程已冻结
      │                                        │
      │  [完全冻结，不执行任何代码]              │  task_for_pid()     → 拿到子进程句柄
      │                                        │  task_threads()     → 拿到线程列表
      │                                        │  thread_get_state() → 读寄存器
      │                                        │  printf PC/SP/FP/x0
      │                                        │
      │  ←──────────── ptrace(PT_CONTINUE) ────┘
      │
      │  execl("ls -l -h")  → 打印文件列表
      ▼
```

此刻子进程停在哪里？子进程刚执行完 `raise(SIGSTOP)`，还没有执行 `execl`。所以读到的 PC 指向 `raise` 系统调用返回后的下一条指令，SP/FP 反映的是当前的栈状态。

## 实现功能

### 1. fork 子进程并附加跟踪

- 子进程调用 `ptrace(PT_TRACE_ME, ...)` 声明允许被父进程跟踪
- 子进程调用 `raise(SIGSTOP)` 主动暂停，等待父进程检查

### 2. 通过 Mach API 获取子进程 task

- 父进程调用 `task_for_pid()` 获取子进程的 Mach task 句柄
- 调用 `task_threads()` 获取子进程的线程列表

### 3. 读取 ARM64 寄存器状态

- 调用 `thread_get_state()` 读取第一个线程的 `arm_thread_state64_t`
- 打印以下关键寄存器：

| 寄存器 | 说明                        | x86 对应 |
|--------|-----------------------------|----------|
| `PC`   | 程序计数器，指向当前指令地址 | RIP      |
| `SP`   | 栈指针                      | RSP      |
| `FP`   | 帧指针（x29）               | RBP      |
| `x0`   | 第一个参数 / 返回值寄存器   | RAX      |

> Apple Silicon 启用了指针认证（PAC），访问 PC/SP/FP 必须使用宏
> `arm_thread_state64_get_pc/sp/fp()` 而非直接读取结构体字段。

### 4. 恢复子进程执行

- 父进程调用 `ptrace(PT_CONTINUE, ...)` 恢复子进程
- 子进程最终执行 `ls -l -h`

### 为什么有用？

这是调试器（如 lldb/gdb）的底层原理——暂停目标进程、检查/修改寄存器和内存、再恢复执行。这个 Demo 只做了"读寄存器"这一步，是实现断点、单步执行等功能的基础。

## 平台要求

- macOS（Apple Silicon，ARM64）
- 需要关闭 SIP 或为可执行文件赋予调试权限，否则 `task_for_pid` 会返回权限错误

## 构建

```bash
cmake -B cmake-build-debug
cmake --build cmake-build-debug
./cmake-build-debug/ptrace_0
```

## 参考

- [linux-沙盒入门，ptrace从0到1](https://cloud.tencent.com/developer/article/1799705)
- `ptrace(2)` man page
- Apple Mach API: `task_for_pid`, `task_threads`, `thread_get_state`
