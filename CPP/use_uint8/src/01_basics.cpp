// 01_basics.cpp
// 演示：uint8_t 的基础声明、取值范围、以及和 char 的符号性区别
//
// 编译：g++ -std=c++17 01_basics.cpp -o 01_basics

#include <cstdint>
#include <cstdio>

void demo_01_basics() {
    printf("========== 1. 基础声明与取值范围 ==========\n");

    uint8_t a = 200;          // 合法：uint8_t 范围是 0~255
    uint8_t b = 255;
    printf("a = %u, b = %u\n", a, b);

    // 注意：printf 打印 uint8_t 时不能直接用 %c 或裸打印，
    // 因为 uint8_t 本质是 unsigned char，%u 需要先转成 unsigned int 才安全，
    // 这里用 %u 是因为整数提升（integer promotion）会自动把它转成 int/unsigned int。

    printf("\n========== 2. 溢出行为（回绕 wrap-around） ==========\n");
    uint8_t c = 255;
    c = c + 1;   // 255 + 1 -> 溢出，无符号类型定义良好地"回绕"到 0
    printf("255 + 1 (uint8_t) = %u  (这是合法的回绕行为，不是未定义行为)\n", c);

    uint8_t d = 0;
    d = d - 1;   // 0 - 1 -> 回绕到 255
    printf("0 - 1 (uint8_t) = %u\n", d);

    printf("\n========== 3. 与 char 的符号性对比（常见 bug 来源） ==========\n");
    // 很多平台上 char 默认是 signed char，取值范围是 -128~127
    char sc = static_cast<char>(200);   // 200 超出 signed char 范围，实现定义/回绕成负数
    uint8_t uc = 200;                    // uint8_t 里 200 就是 200，没有歧义

    printf("char(200) 当作有符号数打印  = %d\n", sc);
    printf("uint8_t(200) 打印           = %u\n", uc);

    if (sc > 100) {
        printf("char 200 > 100 ? 成立\n");
    } else {
        printf("char 200 > 100 ? 不成立 —— 这就是符号性导致的隐蔽 bug！\n");
    }

    if (uc > 100) {
        printf("uint8_t 200 > 100 ? 成立（符合直觉，无歧义）\n");
    }

    printf("\n========== 4. sizeof 确认大小 ==========\n");
    printf("sizeof(uint8_t) = %zu 字节\n", sizeof(uint8_t));

}
