# uint8_t 用法演示项目

这个项目通过 6 示例，演示 `uint8_t` / `uint8_t*` 在 C++ 实际开发中最常见的用法。

## 目录结构

```
uint8_demo/
├── src/
│   ├── 01_basics.cpp          基础：声明、取值范围、与 char 的区别
│   ├── 02_pointer_arith.cpp   指针算术：按字节偏移访问内存
│   ├── 03_protocol_parse.cpp  二进制协议解析（网络包/文件格式常见场景）
│   ├── 04_bitwise_ops.cpp     位运算：标志位、掩码、打包/解包
│   ├── 05_hexdump.cpp         内存十六进制转储（调试利器）
│   └── 06_checksum.cpp        校验和计算 + 简单序列化（ByteBuffer）
├── Makefile
└── README.md
```

## 编译运行

```bash
make          # 编译全部 6 个示例
make run      # 编译并依次运行全部示例
./01_basics   # 也可以单独运行某一个
```

## 每个示例讲了什么

| 文件 | 核心知识点 |
|---|---|
| 01_basics | `uint8_t` 值域 0~255；对比 `char` 的符号性陷阱 |
| 02_pointer_arith | `uint8_t*` 步长为 1 字节；`void*` → `uint8_t*` 转换；越界风险 |
| 03_protocol_parse | 从原始字节流里解析结构化数据（大小端处理、`memcpy` 安全读取） |
| 04_bitwise_ops | 用一个 `uint8_t` 打包 8 个开关状态；位掩码读写 |
| 05_hexdump | 实现一个类似 `xxd`/`hexdump` 的小工具，逐字节打印任意内存内容 |
| 06_checksum | 用 `uint8_t` 数组做简单序列化容器 + CRC 风格校验和 |

建议按顺序阅读源码注释，从最基础的概念到实际工程场景逐步深入。
