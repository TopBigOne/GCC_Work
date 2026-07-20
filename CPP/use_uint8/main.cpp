// main.cpp
// 统一入口：依次运行 01~06 示例

#include <cstdio>

void demo_01_basics();
void demo_02_pointer_arith();
void demo_03_protocol_parse();
void demo_04_bitwise_ops();
void demo_05_hexdump();
void demo_06_checksum();

int main() {
    printf("\n==================== 01_basics ====================\n");
    demo_01_basics();

    printf("\n==================== 02_pointer_arith ====================\n");
    demo_02_pointer_arith();

    printf("\n==================== 03_protocol_parse ====================\n");
    demo_03_protocol_parse();

    printf("\n==================== 04_bitwise_ops ====================\n");
    demo_04_bitwise_ops();

    printf("\n==================== 05_hexdump ====================\n");
    demo_05_hexdump();

    printf("\n==================== 06_checksum ====================\n");
    demo_06_checksum();

    return 0;
}
