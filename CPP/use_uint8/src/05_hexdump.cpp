// 05_hexdump.cpp
// 演示：写一个类似 xxd/hexdump 的小工具，用 uint8_t* 逐字节查看任意内存内容
// 这是调试/逆向/协议分析时非常实用的工具
//
// 编译：g++ -std=c++17 05_hexdump.cpp -o 05_hexdump

#include <cstdint>
#include <cstdio>
#include <cctype>
#include <cstring>
#include <string>

// 核心函数：接收任意起始地址和长度，打印类似 xxd 的输出
// 格式： 偏移地址 | 十六进制字节 | ASCII 可打印字符
void hexDump(const void* data, size_t len) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);  // void* -> uint8_t*
    const size_t bytesPerLine = 16;

    for (size_t offset = 0; offset < len; offset += bytesPerLine) {
        printf("%08zx  ", offset);   // 偏移地址列

        // 十六进制列
        for (size_t i = 0; i < bytesPerLine; ++i) {
            if (offset + i < len) {
                printf("%02x ", bytes[offset + i]);
            } else {
                printf("   ");  // 补齐对齐
            }
            if (i == 7) printf(" ");  // 8 字节一组，中间加空格分隔
        }

        printf(" |");
        // ASCII 列：可打印字符正常显示，不可打印显示成 '.'
        for (size_t i = 0; i < bytesPerLine && offset + i < len; ++i) {
            uint8_t b = bytes[offset + i];
            putchar(std::isprint(b) ? static_cast<char>(b) : '.');
        }
        printf("|\n");
    }
}

void demo_05_hexdump() {
    printf("========== 1. 查看一个字符串常量的原始字节 ==========\n");
    const char* text = "Hello, uint8_t world! 123";
    hexDump(text, std::strlen(text));

    printf("\n========== 2. 查看一个结构体的内存布局 ==========\n");
    struct Config {
        uint32_t id;
        double   ratio;
        char     name[8];
    };
    Config cfg{ 0xCAFEBABE, 3.14159, "cfgname" };
    hexDump(&cfg, sizeof(cfg));

    printf("\n========== 3. 查看一段人工构造的二进制数据 ==========\n");
    uint8_t raw[20];
    for (size_t i = 0; i < sizeof(raw); ++i) {
        raw[i] = static_cast<uint8_t>(i * 13);  // 随便填一些值，制造非打印字符
    }
    hexDump(raw, sizeof(raw));

}
