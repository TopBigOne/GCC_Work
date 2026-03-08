# 内存对齐（Memory Alignment）学习文档

## 1. 什么是内存对齐？

CPU 访问内存时，并不是一次读取 1 个字节，而是按**固定宽度的"总线宽度"**批量读取（如 4 字节、8 字节）。

**内存对齐**就是要求变量存放的起始地址必须是其自身大小的整数倍：

| 类型       | 大小   | 对齐要求（起始地址必须是…的倍数） |
|------------|--------|-----------------------------------|
| `char`     | 1 字节 | 1                                 |
| `short`    | 2 字节 | 2                                 |
| `int`      | 4 字节 | 4                                 |
| `double`   | 8 字节 | 8                                 |
| `pointer`  | 8 字节 | 8（64 位系统）                    |

---

## 2. 为什么需要内存对齐？

### 性能原因
CPU 读取内存时按对齐的块读取。若一个 `int` 跨越两个块的边界，CPU 需要读取两次再拼合，效率低下。

```
未对齐（跨边界）：
地址:  0    1    2    3    4    5    6    7
      [--] [int的字节0][字节1][字节2][字节3] ...
                           ↑ int 从地址1开始，跨越两个4字节块 → 需读取两次

已对齐：
地址:  0    1    2    3    4    5    6    7
      [int的字节0][字节1][字节2][字节3] ...
       ↑ int 从地址0开始，一次读取完毕
```

### 硬件强制
某些 CPU（如 ARM）对未对齐访问直接抛出 **硬件异常（Bus Error）**，程序崩溃。

---

## 3. 结构体中的内存对齐

编译器会在结构体成员之间自动插入**填充字节（padding）**，使每个成员满足对齐要求。

### 示例（本项目中的 `int_char`）

```c
struct int_char {
    int  x;  // 4 字节，对齐到 4
    char y;  // 1 字节，对齐到 1
};
```

直觉上认为大小是 4 + 1 = **5 字节**，但实际是 **8 字节**：

```
地址偏移:  0    1    2    3    4    5    6    7
          [  x (int, 4字节)  ] [ y ] [pad] [pad] [pad]
```

**规则：结构体整体大小必须是其最大对齐成员的整数倍。**
这里最大成员是 `int`（4 字节），所以总大小对齐到 4 的倍数 → 8 字节。

### 更复杂的例子

```c
struct Bad {   // 成员顺序不好
    char  a;   // 1 字节 → 偏移 0
               // [pad] [pad] [pad]  ← 3 字节填充
    int   b;   // 4 字节 → 偏移 4
    char  c;   // 1 字节 → 偏移 8
               // [pad] [pad] [pad]  ← 3 字节填充
};             // 总大小: 12 字节

struct Good {  // 调整顺序后
    int   b;   // 4 字节 → 偏移 0
    char  a;   // 1 字节 → 偏移 4
    char  c;   // 1 字节 → 偏移 5
               // [pad] [pad]        ← 2 字节填充
};             // 总大小: 8 字节
```

**技巧：把大类型成员放前面，小类型放后面，可以减少填充字节。**

---

## 4. 通过指针查看内存布局

用 `uint8_t *` 可以逐字节查看任意数据在内存中的实际存储：

```c
int value = 0x12345678;
uint8_t *p = (uint8_t *)&value;

printf("字节0: 0x%02X\n", p[0]);  // 小端系统: 0x78
printf("字节1: 0x%02X\n", p[1]);  // 小端系统: 0x56
printf("字节2: 0x%02X\n", p[2]);  // 小端系统: 0x34
printf("字节3: 0x%02X\n", p[3]);  // 小端系统: 0x12
```

这也引出了下一个概念：**字节序**。

---

## 5. 字节序（Endianness）

同样的数据，不同架构下字节存储顺序不同：

| 字节序   | 描述                       | 代表架构          |
|----------|----------------------------|-------------------|
| 小端（Little-Endian） | 低位字节存在低地址 | x86, ARM（默认）  |
| 大端（Big-Endian）    | 高位字节存在低地址 | MIPS, 网络协议    |

```
数值 0x12345678 在内存中：

小端：[78] [56] [34] [12]  ← 低地址 → 高地址
大端：[12] [34] [56] [78]  ← 低地址 → 高地址
```

---

## 6. `offsetof` 和 `sizeof` 工具

```c
#include <stddef.h>

struct Example {
    char  a;
    int   b;
    short c;
};

// 查看每个成员的偏移量
printf("a offset: %zu\n", offsetof(struct Example, a));  // 0
printf("b offset: %zu\n", offsetof(struct Example, b));  // 4
printf("c offset: %zu\n", offsetof(struct Example, c));  // 8
printf("总大小:   %zu\n", sizeof(struct Example));        // 12
```

---

## 7. 手动控制对齐

### 使用 `#pragma pack` 取消填充（谨慎使用）

```c
#pragma pack(1)       // 按 1 字节对齐（取消填充）
struct Packed {
    char a;           // 偏移 0
    int  b;           // 偏移 1（不再填充！）
    char c;           // 偏移 5
};                    // 总大小: 6 字节
#pragma pack()        // 恢复默认
```

> 常用于网络协议、文件格式等需要精确控制内存布局的场景。

### 使用 `__attribute__((aligned(N)))` 指定对齐

```c
int x __attribute__((aligned(16)));  // x 的地址强制对齐到 16 字节
```

---

## 8. 实践：验证结构体大小

在本项目中运行 `test_one()` 即可验证：

```c
void test_one() {
    printf("finally size : %lu\n", sizeof(INT_CHAR));  // 输出 8，而不是 5
}
```

---

## 9. 总结

| 要点 | 说明 |
|------|------|
| 对齐规则 | 变量地址必须是自身大小的整数倍 |
| 结构体填充 | 编译器自动插入 padding，整体大小对齐最大成员 |
| 性能影响 | 合理排列成员顺序可减少内存浪费 |
| 查看布局 | 用 `uint8_t*` 逐字节查看，`offsetof` 查看偏移 |
| 强制控制 | `#pragma pack` 或 `__attribute__((aligned))` |

---

## 参考

- [cppreference - Object representation](https://en.cppreference.com/w/c/language/object)
- [cppreference - offsetof](https://en.cppreference.com/w/c/types/offsetof)
