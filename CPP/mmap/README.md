# Linux mmap 学习大纲

## 阶段 0：前置知识

| 概念 | 要点 |
|------|------|
| 虚拟内存基础 | 虚拟地址 vs 物理地址、页（page，通常 4KB）、页表、MMU |
| 进程地址空间布局 | 代码段、数据段、堆、栈、mmap 区域 |
| 文件 I/O 基础 | `open` / `read` / `write` / `close` / `fstat` |
| page cache | 内核如何缓存文件内容 |
| 缺页中断（page fault） | 为什么 mmap 是"按需加载"的 |

---

## 阶段 1：基本用法

### 1.1 接口

```c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
```

### 1.2 参数逐个吃透

- **prot**：`PROT_READ` / `PROT_WRITE` / `PROT_EXEC` / `PROT_NONE`
- **flags**：
  - `MAP_SHARED` — 修改会写回文件，进程间可见
  - `MAP_PRIVATE` — 写时复制 COW，修改不影响文件
  - `MAP_ANONYMOUS` — 不关联文件，纯内存
  - `MAP_FIXED`、`MAP_POPULATE`、`MAP_HUGETLB` 等进阶标志
- **offset** 必须按页对齐，**length** 内部会向上取整到页
- 失败返回 `MAP_FAILED`（不是 `NULL`）

### 1.3 动手练习

- [x] 用 mmap 读取一个文件并打印内容
- [x] 用 mmap 修改文件内容（`MAP_SHARED` + `PROT_WRITE`），再用 cat 验证
- [x] 对比 `MAP_PRIVATE` 与 `MAP_SHARED` 修改后的区别

> 代码：[`src/stage1/`](src/stage1/)　·　学习笔记：[stage1_basic.md](src/stage1/stage1_basic.md)

---

## 阶段 2：配套系统调用

| 函数 | 学习重点 |
|------|---------|
| `munmap` | 解除映射，部分解除的行为 |
| `msync` | `MS_SYNC` / `MS_ASYNC`，何时数据真正落盘 |
| `mprotect` | 运行时修改页权限 |
| `madvise` | `MADV_SEQUENTIAL` / `MADV_WILLNEED` / `MADV_DONTNEED` 等访问提示 |
| `mremap` | 调整映射大小（Linux only） |
| `mlock` / `munlock` | 锁定内存，防止换出 |
| `ftruncate` | 映射前先给文件设定大小 |

> **常见坑**：映射超出文件大小的区域，访问会触发 `SIGBUS`。

> 代码：[`src/stage2/`](src/stage2/)　·　学习笔记：[stage2_syscalls.md](src/stage2/stage2_syscalls.md)

---

## 阶段 3：匿名映射与内存分配

- `MAP_ANONYMOUS | MAP_PRIVATE`：自己实现大块内存申请
- `malloc` 与 mmap 的关系：大块分配（默认约 128KB 以上）走 mmap，小块走 brk
- 零页与写时复制：为什么匿名映射刚申请时不占物理内存
- 用 `/proc/self/maps`、`/proc/self/smaps` 观察进程的映射情况
- **实践**：写一个简单的 Arena / 内存池，底层用 mmap

> 代码：[`src/stage3/`](src/stage3/)　·　学习笔记：[stage3_anonymous.md](src/stage3/stage3_anonymous.md)

---

## 阶段 4：进程间共享内存

- 父子进程 `fork` 后共享匿名 `MAP_SHARED` 映射
- 无亲缘进程：`shm_open` + mmap（POSIX 共享内存，对应 `/dev/shm`）
- 同步问题：共享内存本身不提供同步，需配合信号量、`pthread_mutex`（`PTHREAD_PROCESS_SHARED`）或原子操作
- **实践**：两个进程通过共享内存实现生产者/消费者环形缓冲区

> 代码：[`src/stage4/`](src/stage4/)　·　学习笔记：[stage4_shared_mem.md](src/stage4/stage4_shared_mem.md)

---

## 阶段 5：设备与驱动层 mmap

- `/dev/mem`、`/dev/fb0`（framebuffer）：把硬件地址映射到用户空间
- V4L2 摄像头：`VIDIOC_REQBUFS` + `VIDIOC_QUERYBUF` + mmap 取得内核视频缓冲
- DRM/KMS、DMA-BUF：dma-buf 的 fd 如何被 mmap
- 驱动侧：`file_operations.mmap`、`remap_pfn_range`、`vm_operations_struct`
- 缓存一致性：cached / uncached / write-combine 映射，以及 DMA 场景下的同步

> 代码：[`src/stage5/`](src/stage5/)　·　学习笔记：[stage5_device.md](src/stage5/stage5_device.md)

---

## 阶段 6：Android / NDK 场景（先不学）

- ashmem / `memfd_create` / `AHardwareBuffer`：Android 共享内存的演进
- `SharedMemory`（Java）与 native 层 fd 传递，再 mmap
- Binder 传递 fd 实现跨进程大数据共享
- mmap 加载大文件、模型文件、资源文件（比一次性 read 省内存）
- 性能分析：RSS / PSS / VSS 的区别，`dumpsys meminfo` 中 mmap 类内存归属

---

## 阶段 7：内核原理（进阶）

- `vm_area_struct`（VMA）与 `mm_struct`
- mmap 系统调用的内核路径：`do_mmap` → 建立 VMA → 缺页时才真正分配
- 缺页处理：文件映射读入 page cache、匿名页分配、COW
- 页回收与 `MAP_SHARED` 脏页回写（writeback）
- 透明大页（THP）、`MAP_HUGETLB` 与 NUMA 影响
- 安全相关：ASLR、W^X、`mprotect` 与 JIT

> 代码：[`src/stage7/`](src/stage7/)　·　学习笔记：[stage7_kernel.md](src/stage7/stage7_kernel.md)

---

## 阶段 8：性能与取舍

- **mmap vs read/write**：什么情况下 mmap 更快，什么情况下反而更慢
- 优势：避免一次用户态拷贝、随机访问方便、多进程共享
- 劣势：缺页开销、TLB 压力、I/O 错误以信号（`SIGBUS`）形式出现、文件被截断的风险
- 顺序读大文件 vs 随机读的 benchmark
- 多线程访问同一映射的注意事项
- 用 `perf`、`strace`、`/proc/<pid>/status`（VmRSS）观察实际行为

> 代码：[`src/stage8/`](src/stage8/)　·　学习笔记：[stage8_benchmark.md](src/stage8/stage8_benchmark.md)

---

## 阶段 9：综合实战项目

| 项目 | 说明 |
|------|------|
| mini cp / grep | 用 mmap 实现，并和 read 版本做性能对比 |
| 共享内存环形队列 | 跨进程无锁（或加锁）通信 |
| 简易 KV 存储 | mmap 持久化文件 + 追加写，类似 LevelDB / LMDB 的雏形 |
| V4L2 抓图程序 | mmap 取摄像头帧并保存为文件 |
| 自制内存池 / arena allocator | 匿名映射为底层 |

> 代码：[`src/stage9/`](src/stage9/)　·　学习笔记：[stage9_projects.md](src/stage9/stage9_projects.md)

---

## 推荐资料

- `man 2 mmap`、`man 2 madvise`、`man 7 shm_overview`
- 《UNIX 环境高级编程》（APUE）：内存映射 I/O、共享内存章节
- 《Linux 内核设计与实现》《深入理解 Linux 内核》：内存管理部分
- 内核文档：`Documentation/admin-guide/mm/`
- 开源代码参考：LMDB（mmap 数据库经典实现）、glibc malloc

---

## 项目结构

```
mmap/
├── CMakeLists.txt
├── README.md
├── hello.txt                    # 测试用文件
├── main.cpp                     # 入口，菜单选择各阶段 demo
├── doc/                         # 跨阶段的专题文档（系统调用全景图等）
└── src/
    ├── mmap_wrapper.hpp         # RAII 封装 + Builder 模式（header-only，各阶段共用）
    ├── stage1/                  # 阶段 1：基本用法
    │   ├── stage1_basic.h       #   声明 + Doxygen 注释
    │   ├── stage1_basic.cpp     #   实现
    │   ├── stage1_basic.md      #   学习笔记（知识点讲解 + 运行输出解读 + 练习）
    │   └── images/              #   笔记中的 SVG 图
    ├── stage2/                  # 阶段 2：配套系统调用（结构同上）
    ├── stage3/                  # 阶段 3：匿名映射与 Arena
    ├── stage4/                  # 阶段 4：进程间共享内存
    ├── stage5/                  # 阶段 5：设备 mmap
    ├── stage7/                  # 阶段 7：内核观测
    ├── stage8/                  # 阶段 8：性能基准测试
    └── stage9/                  # 阶段 9：综合实战
```

每个阶段一个文件夹：`.h` 只放声明，每个函数都带 Doxygen 注释（干什么、参数、返回值、实际应用）；`.cpp` 放实现。模板（如 `RingBuffer`、`ArenaAllocator::create`）必须留在 `.h` 中。

## 设计模式

| 模式 | 应用场景 |
|------|---------|
| **RAII** | `MmapRegion`、`FileDescriptor`、`SharedMemory` — 自动管理映射/fd/shm 生命周期 |
| **Builder** | `MmapBuilder` — 链式配置 mmap 参数 |
| **Template Method** | `Benchmark` 基类 — 统一 setup/run/teardown 流程 |
| **Strategy** | `SyscallDemo` — 可切换的 madvise 策略 |
| **Observer** | `MemoryObserver` — 监测内存映射变化 |
