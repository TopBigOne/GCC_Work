# test_bit

## 项目目的

练习 C 语言中的位操作：将一个 16 位整数拆分为两个 8 位字节存入缓冲区，再将其还原。

## 核心知识点

### 1. 拆分（高低字节分离）

```c
int spsLength = 300; // 0x012C

uint8_t high = (spsLength >> 8) & 0xFF; // 高字节: 0x01 = 1
uint8_t low  = (spsLength)      & 0xFF; // 低字节: 0x2C = 44
```

### 2. 存入 `uint8_t` 缓冲区

```c
pkt->m_body[0] = high;
pkt->m_body[1] = low;
```

### 3. 还原（高低字节合并）

```c
int full = (pkt->m_body[0] << 8) | pkt->m_body[1]; // 还原为 300
```

## 踩过的坑

| 问题 | 原因 | 修复 |
|------|------|------|
| `printf` 打印 44 而不是 300 | 把 300 直接赋给 `char`，发生截断（300 & 0xFF = 44） | 先拆字节再分别存储 |
| 变量名 `packet` 遮蔽类型名 | `sizeof(packet)` 变成 `sizeof(char*)`，大小不对 | 改变量名为 `pkt` |
| `m_body` 野指针崩溃 | 只声明指针，未 `malloc` 分配内存 | 显式 malloc 后再使用 |
| `char *m_body` 符号问题 | `char` 有符号，读取时可能符号扩展导致值错误 | 改为 `uint8_t *m_body` |
| 内存泄漏 | malloc 后没有 free | 用完后 `free(pkt->m_body)` 和 `free(pkt)` |
