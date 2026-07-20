// 04_bitwise_ops.cpp
// 演示：用一个 uint8_t 打包 8 个布尔标志位（常见于配置项、权限位、状态机）
//
// 编译：g++ -std=c++17 04_bitwise_ops.cpp -o 04_bitwise_ops

#include <cstdint>
#include <cstdio>

// 用具名常量表示每一位的含义，比裸数字 1,2,4,8... 可读性更好
enum Flags : uint8_t {
    FLAG_READ    = 1 << 0,  // 0b00000001
    FLAG_WRITE   = 1 << 1,  // 0b00000010
    FLAG_EXECUTE = 1 << 2,  // 0b00000100
    FLAG_HIDDEN  = 1 << 3,  // 0b00001000
    FLAG_SYSTEM  = 1 << 4,  // 0b00010000
};

void printBinary(uint8_t v) {
    for (int i = 7; i >= 0; --i) {
        printf("%d", (v >> i) & 1);
    }
}

void demo_04_bitwise_ops() {
    printf("========== 1. 设置标志位（按位或 |=） ==========\n");
    uint8_t permission = 0;
    permission |= FLAG_READ;
    permission |= FLAG_WRITE;

    printf("permission = ");
    printBinary(permission);
    printf(" (十进制 %u)\n", permission);

    printf("\n========== 2. 检查某个标志位是否被设置（按位与 &） ==========\n");
    if (permission & FLAG_READ) {
        printf("有读权限\n");
    }
    if (permission & FLAG_EXECUTE) {
        printf("有执行权限\n");
    } else {
        printf("没有执行权限\n");
    }

    printf("\n========== 3. 清除某个标志位（按位与 & 加取反 ~） ==========\n");
    permission &= ~FLAG_WRITE;   // 清掉写权限
    printf("清除写权限后 = ");
    printBinary(permission);
    printf(" (十进制 %u)\n", permission);

    printf("\n========== 4. 翻转某个标志位（按位异或 ^） ==========\n");
    permission ^= FLAG_EXECUTE;  // 没有则加上，有则去掉
    printf("翻转执行权限后 = ");
    printBinary(permission);
    printf(" (十进制 %u)\n", permission);

    printf("\n========== 5. 用高低 4 位分别存两个 0~15 的小数值（打包/解包） ==========\n");
    uint8_t major = 3, minor = 7;   // 假设各自范围是 0~15
    uint8_t packedVersion = (major << 4) | (minor & 0x0F);
    printf("打包后 packedVersion = ");
    printBinary(packedVersion);
    printf(" (十进制 %u)\n", packedVersion);

    uint8_t unpackedMajor = (packedVersion >> 4) & 0x0F;
    uint8_t unpackedMinor = packedVersion & 0x0F;
    printf("解包结果: major=%u, minor=%u\n", unpackedMajor, unpackedMinor);

}
