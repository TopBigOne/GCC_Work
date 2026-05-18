# ptrace_0 - macOS ARM64 进程跟踪入门

## 简介

本项目是 `ptrace` + Mach API 的入门学习 Demo，运行在 **macOS Apple Silicon（ARM64）** 平台。

演示了如何通过 `ptrace` 和 macOS Mach 内核接口，在父进程中读取子进程的 CPU 寄存器状态。

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

| 寄存器 | 说明 | x86 对应 |
|--------|------|----------|
| `PC`   | 程序计数器，指向当前指令地址 | RIP |
| `SP`   | 栈指针 | RSP |
| `FP`   | 帧指针（x29） | RBP |
| `x0`   | 第一个参数 / 返回值寄存器 | RAX |

> Apple Silicon 启用了指针认证（PAC），访问 PC/SP/FP 必须使用宏
> `arm_thread_state64_get_pc/sp/fp()` 而非直接读取结构体字段。

### 4. 恢复子进程执行

- 父进程调用 `ptrace(PT_CONTINUE, ...)` 恢复子进程
- 子进程最终执行 `ls -l -h`

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
