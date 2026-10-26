# new 的三种写法：普通 new、placement new、nothrow new

stage4 里有这样一行：

```cpp
Ring *ring = new (shm.data()) Ring();
```

`new` 和类型名中间多了一对括号。这篇从这里出发，把 `new` 的三种常见写法讲清楚：

| 写法                     | 名字                      | 一句话                                            |
|--------------------------|---------------------------|---------------------------------------------------|
| `new T()`                | 普通 new                  | 去堆上申请内存，再构造对象                        |
| `new (地址) T()`         | placement new（定位 new） | **不申请内存**，在给定地址上构造对象              |
| `new (std::nothrow) T()` | nothrow new               | 去堆上申请内存，失败时返回 `nullptr` 而不是抛异常 |

![new 的三种写法](images/new-three-forms.svg)

---

## 1. 先弄清楚：一个 new 表达式其实做了两件事

```cpp
Box *p = new Box(1);
```

编译器会把它拆成两步：

```cpp
void *mem = operator new(sizeof(Box));   // ① 调用 operator new 函数，拿到一块原始内存
Box *p = 构造 Box(1) 在 mem 上;            // ② 在这块内存上调用 Box 的构造函数
```

注意区分两个名字：

- **`new` 表达式**：就是你写的 `new Box(1)`，负责①和②两步。
- **`operator new` 函数**：只负责第 ① 步，相当于 C++ 版的 `malloc`，只分配内存，不调用构造函数。

`delete` 也一样，是两步反过来：

```cpp
delete p;
// 相当于：
p->~Box();                 // ① 调用析构函数
operator delete(p);        // ② 把内存还回去
```

**三种写法的区别全部在第 ① 步**：内存从哪来、拿不到内存时怎么办。第 ② 步（调用构造函数）都是一样的。

---

## 2. new 后面的括号是什么

```
new ( 额外参数… )  T ( 构造参数… )
    └────┬─────┘    └─────┬─────┘
   交给 operator new     交给构造函数
   决定内存从哪来        决定对象怎么初始化
```

- 类型**后面**的括号 `T(...)`：大家都熟悉，是**构造函数的参数**。
- `new` **后面**、类型**前面**的括号 `(...)`：是传给 **`operator new` 的额外参数**。

编译器会把第一个括号里的东西，作为第 2、3……个参数传给 `operator new`（第 1 个参数永远是要分配的字节数 `sizeof(T)`）：

```cpp
new T()                      →  operator new(sizeof(T))
new (addr) T()               →  operator new(sizeof(T), addr)
new (std::nothrow) T()       →  operator new(sizeof(T), std::nothrow)
```

标准库 `<new>` 里已经提供了这几个版本的 `operator new`，所以这三种写法开箱即用。

### 验证：自己写一个带额外参数的 operator new

为了证明"括号里的东西会传给 operator new"，可以自己重载一个：

```cpp
#include <cstdio>
#include <new>

// 自定义一个带额外参数的 operator new：括号里写什么，就传给这里的第 2 个及以后的参数
void *operator new(std::size_t size, const char *who) {
    printf("  operator new(size=%zu, who=\"%s\")\n", size, who);
    return ::operator new(size);   // 真正的分配还是交给全局默认版本
}

struct Point { double x, y; };

int main() {
    Point *p = new ("main 函数") Point{1.0, 2.0};
    printf("  p = %p, x=%.1f y=%.1f\n", (void *)p, p->x, p->y);
    ::operator delete(p);
}
```

本机输出：

```
  operator new(size=16, who="main 函数")
  p = 0x125605be0, x=1.0 y=2.0
```

`new ("main 函数") Point{...}` 括号里的字符串，确实被传给了 `operator new` 的第二个参数 `who`；第一个参数 `size=16` 就是 `sizeof(Point)`。

placement new 和 nothrow new 就是标准库预先写好的两个这样的重载。

---

## 3. 普通 new

```cpp
Box *p = new Box(1);     // 堆上申请 + 构造
delete p;                // 析构 + 释放
```

- 内存来自**堆**。
- 内存不够时，**抛出 `std::bad_alloc` 异常**；如果没有 `try/catch` 接住，程序终止。
- 用 `delete` 释放。数组用 `new T[n]` / `delete[] p`，必须配对使用。

---

## 4. placement new（定位 new）

### 写法

```cpp
#include <new>                          // 必须包含

void *addr = ...;                       // 一块已经准备好的内存
T *p = new (addr) T(构造参数);           // 在 addr 上构造 T
...
p->~T();                                // 用完：手动调用析构函数
// 内存交还给它原来的主人（不要 delete）
```

### 它做了什么

标准库里的实现就这么一行：

```cpp
void *operator new(std::size_t, void *where) noexcept { return where; }
```

**不申请任何内存，你给的地址原样返回**。然后编译器在这个地址上调用构造函数。

所以 placement new = **只做第 ② 步（构造），跳过第 ① 步（分配）**。

### 为什么需要它

当**内存已经有了，但上面还没有对象**的时候。典型场景：

| 场景 | 内存从哪来 | 本项目中的例子 |
|---|---|---|
| 进程间共享 | `shm_open` + `mmap` 得到的共享内存 | stage4：[`new (shm.data()) Ring()`](../src/stage4/stage4_shared_mem.cpp#L181) |
| 内存池 / Arena | 一次 mmap 拿到的大块内存，切成小块用 | stage3：[`ArenaAllocator::create`](../src/stage3/stage3_anonymous.h#L128) 里的 `new (ptr) T{...}` |
| 标准库容器 | `std::vector` 先 `reserve` 一大块，`push_back` 时在空位上构造元素 | `std::vector`、`std::optional`、`std::variant` 内部都在用 |
| 嵌入式 | 硬件寄存器、指定的静态缓冲区 | — |

以 stage4 为例：环形队列必须放在共享内存里，fork 出来的子进程和父进程才能看到同一个队列。如果用普通 `new Ring()`，队列会建在堆上，而堆是每个进程私有的（fork 后写时复制），子进程 push 的数据父进程看不到。

### 使用条件（编译器不检查，全靠你保证）

| 条件 | 违反了会怎样 |
|---|---|
| ① **空间够大**：至少 `sizeof(T)` 字节 | 构造时写出边界，覆盖别的数据 |
| ② **地址对齐**：地址是 `alignof(T)` 的整数倍 | 有的 CPU 直接崩溃，原子操作可能出错 |
| ③ **内存可写、有效**：不是只读页、不是已释放的内存 | SIGSEGV / SIGBUS |
| ④ **上面没有"活着"的对象**：有的话先析构 | 旧对象持有的资源泄漏 |

stage4 中，共享内存大小就是 `sizeof(Ring)`，mmap 返回的地址按页对齐，以 `PROT_READ | PROT_WRITE` 映射，又是新建的，四条都满足。

### 释放：只能手动析构，绝不能 delete

```cpp
alignas(std::string) unsigned char buf[sizeof(std::string)];
std::string *s = new (buf) std::string("hello");

s->~basic_string();     // ✓ 调用析构函数，释放 string 内部的堆内存
// delete s;            // ✗ 错！buf 在栈上，不是 operator new 给的，delete 会让程序崩溃
```

原则：**谁分配的内存谁释放，谁构造的对象谁析构**。placement new 只负责构造，析构就要你自己调用；内存还给原主人：栈上的 `buf` 函数结束自动回收，共享内存由 `SharedMemory` 析构时 munmap。

stage4 没有写 `ring->~Ring()`，是因为 `Ring` 的成员只有原子整数和 `int` 数组，析构函数什么都不做。像 `std::string`、`std::vector` 这样内部还持有资源的类型，**必须**手动析构。

### 不要对数组用 placement new

```cpp
new (buf) T[10];    // 能编译，但不推荐
```

C++20 之前，数组形式的 new 可能在开头额外占几个字节记录元素个数，你准备的空间可能不够。需要构造多个对象时，用循环逐个 placement new，或者用 `std::uninitialized_default_construct_n` 等标准库函数。

### C++20 的替代写法

```cpp
T *p = std::construct_at(static_cast<T *>(addr), 构造参数);   // 等价于 new (addr) T(构造参数)
std::destroy_at(p);                                          // 等价于 p->~T()
```

本项目使用 C++17，所以仍用 placement new。

---

## 5. nothrow new

### 写法

```cpp
#include <new>

int *p = new (std::nothrow) int[1000];
if (!p) {
    // 内存不够，自己处理
}
...
delete[] p;
```

`std::nothrow` 是标准库定义的一个"标记"对象（类型是 `std::nothrow_t`），它本身没有任何数据，作用只是让编译器选中下面这个版本的 `operator new`：

```cpp
void *operator new(std::size_t size, const std::nothrow_t &) noexcept;   // 失败时返回 nullptr
```

### 和普通 new 的唯一区别：失败时的表现

| | 普通 new | nothrow new |
|---|---|---|
| 内存从哪来 | 堆 | 堆 |
| 内存不够时 | 抛出 `std::bad_alloc` | **返回 `nullptr`** |
| 调用方要做什么 | 需要处理的话写 `try/catch` | **必须检查返回值** |
| 释放 | `delete` | `delete`（一样） |

### 什么时候用

- 项目**禁用了 C++ 异常**（编译时加了 `-fno-exceptions`），常见于嵌入式、游戏引擎，以及 Chromium、LLVM 这类默认关闭异常编译的大型 C++ 项目。
- 想用 C 语言风格的"检查返回值"来处理错误，比如申请一个很大的缓冲区，失败了就换小一点的再试。

### 两个要注意的地方

1. **它只管"分配失败"**。内存分配成功后，如果**构造函数**抛异常，异常照样会抛出来。
2. **在 Linux 上，"申请成功"不代表"真有内存"**。Linux 默认允许内存超额分配（overcommit）：申请时只是登记虚拟地址，真正写入时才分配物理页（和 mmap 的按需分配是同一个机制）。所以中等大小的申请几乎不会失败，真正内存不足时，进程更可能在**写入时**被 OOM killer 杀掉。nothrow new 能防住的，主要是"申请的大小离谱"这类情况。

---

## 6. 本机实测：三种写法放在一起

```cpp
#include <cstdio>
#include <new>

struct Box {
    int value;
    explicit Box(int v) : value(v) { printf("    Box(%d) 构造，this = %p\n", v, (void *)this); }
    ~Box() { printf("    ~Box() 析构，this = %p\n", (void *)this); }
};

int main() {
    // 故意申请一个不可能成功的大小（约 1 EB）
    const size_t huge = size_t(1) << 60;

    printf("① 普通 new：new Box(1)\n");
    Box *a = new Box(1);
    delete a;

    printf("\n   普通 new 失败时：\n");
    try {
        char *p = new char[huge];
        printf("    意外成功 %p\n", (void *)p);
        delete[] p;
    } catch (const std::bad_alloc &e) {
        printf("    抛出异常 std::bad_alloc: %s\n", e.what());
    }

    printf("\n② placement new：new (buf) Box(2)\n");
    alignas(Box) unsigned char buf[sizeof(Box)];
    printf("    buf = %p\n", (void *)buf);
    Box *b = new (buf) Box(2);
    b->~Box();                                   // 手动析构，不能 delete

    printf("\n③ nothrow new：new (std::nothrow) char[huge]\n");
    char *c = new (std::nothrow) char[huge];
    printf("    返回 %p（%s）\n", (void *)c, c ? "成功" : "失败，返回 nullptr，没有抛异常");
    delete[] c;                                  // delete nullptr 是安全的空操作

    printf("\n   nothrow new 成功时：new (std::nothrow) Box(3)\n");
    Box *d = new (std::nothrow) Box(3);
    delete d;                                    // 内存来自堆，正常 delete
}
```

输出（macOS，Apple Silicon）：

```
① 普通 new：new Box(1)
    Box(1) 构造，this = 0x138605f20
    ~Box() 析构，this = 0x138605f20

   普通 new 失败时：
    抛出异常 std::bad_alloc: std::bad_alloc

② placement new：new (buf) Box(2)
    buf = 0x16b566134
    Box(2) 构造，this = 0x16b566134
    ~Box() 析构，this = 0x16b566134

③ nothrow new：new (std::nothrow) char[huge]
    返回 0x0（失败，返回 nullptr，没有抛异常）

   nothrow new 成功时：new (std::nothrow) Box(3)
    Box(3) 构造，this = 0x138605cb0
    ~Box() 析构，this = 0x138605cb0
```

对照着看：

- ① 和 ③ 成功时，对象地址都是 `0x1386…`，在**堆**上。
- ② 的对象地址 `0x16b566134` **正好等于 `buf` 的地址**，构造和析构都发生在 `buf` 上。
- 同样申请 1 EB：普通 new **抛异常**，nothrow new **返回 0x0**。

---

## 7. 常见错误

| 错误写法 | 问题 | 正确做法 |
|---|---|---|
| `new (buf) T(); delete p;` | 把不是 operator new 给的内存交给堆释放，崩溃 | `p->~T();`，内存交还原主人 |
| placement new 后忘了析构 | `std::string` 等成员持有的资源泄漏 | 用完调用 `p->~T()` |
| `char buf[sizeof(T)]; new (buf) T();` | `buf` 不一定满足 `T` 的对齐要求 | `alignas(T) unsigned char buf[sizeof(T)];` |
| 在同一地址上反复 placement new | 旧对象没析构，资源泄漏 | 先 `p->~T()`，再 placement new |
| `T *p = new (std::nothrow) T(); p->...` | 没检查 `nullptr` | `if (!p) { 处理失败 }` |
| `new T[n]` 配 `delete p` | 数组和单个对象的 new/delete 混用，未定义行为 | `new T[n]` 配 `delete[] p` |

---

## 8. 速查

| | `new T()` | `new (addr) T()` | `new (std::nothrow) T()` |
|---|---|---|---|
| 头文件 | 无 | `<new>` | `<new>` |
| 内存来源 | 堆 | **你给的地址** | 堆 |
| 分配失败 | 抛 `std::bad_alloc` | 不分配，不会失败 | 返回 `nullptr` |
| 调用构造函数 | ✓ | ✓ | 成功时 ✓ |
| 释放 | `delete p` | **`p->~T()`，不能 delete** | `delete p` |
| 典型用途 | 日常创建对象 | 共享内存、内存池、容器内部 | 禁用异常的代码 |

相关文档：[stage4 学习笔记](../src/stage4/stage4_shared_mem.md)（共享内存与环形队列）、[4-讲讲{}:块作用域.md](4-讲讲{}:块作用域.md)（析构时机）。
