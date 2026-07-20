// 02_pointer_arith.cpp
// 演示：uint8_t* 作为"万能字节指针"，如何做精确到字节的指针算术
//
// 编译：g++ -std=c++17 02_pointer_arith.cpp -o 02_pointer_arith

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>

struct Point3D {
    float x, y, z;
};

void demo_02_pointer_arith() {
    printf("========== 1. void* 必须转成 uint8_t* 才能做字节偏移 ==========\n");

    void* raw = std::malloc(64);
    // void* 不能直接 + 1（编译器不知道每个元素多大，做算术是非标准扩展）
    uint8_t* bytes = static_cast<uint8_t*>(raw);

    // 现在可以按字节精确偏移
    uint8_t* p10 = bytes + 10;   // 正好偏移 10 个字节
    printf("bytes 地址     = %p\n", (void*)bytes);
    printf("bytes+10 地址  = %p  (正好差 10)\n", (void*)p10);

    std::free(raw);

    printf("\n========== 2. 对比：int* 的偏移是按 sizeof(int) 走的 ==========\n");
    int arr[5] = {0};
    int* ip = arr;
    int* ip3 = ip + 3;
    printf("int* 每步偏移 %zu 字节, ip+3 跳过了 %ld 字节\n",
           sizeof(int), (long)((uint8_t*)ip3 - (uint8_t*)ip));

    printf("\n========== 3. 结构体按字节访问其内存布局 ==========\n");
    Point3D pt{1.5f, 2.5f, 3.5f};
    uint8_t* structBytes = reinterpret_cast<uint8_t*>(&pt);

    printf("Point3D 占 %zu 字节，逐字节打印其内存内容（十六进制）：\n", sizeof(Point3D));
    for (size_t i = 0; i < sizeof(Point3D); ++i) {
        printf("%02x ", structBytes[i]);
        if ((i + 1) % 4 == 0) printf(" ");  // 每 4 字节（一个 float）加个空格
    }
    printf("\n(可以看到分成 3 组，每组 4 字节，对应 x, y, z 三个 float)\n");

    printf("\n========== 4. 用 uint8_t* 在一大块内存中定位子结构（协议解析常见模式） ==========\n");
    uint8_t buffer[16] = {0};
    // 假设 buffer 前 4 字节是 header(magic number)，后面是 payload
    uint32_t magic = 0xDEADBEEF;
    std::memcpy(buffer, &magic, sizeof(magic));   // 写入 header

    uint8_t* payload = buffer + sizeof(uint32_t);  // 跳过 header，指向 payload 起始处
    size_t payloadLen = sizeof(buffer) - sizeof(uint32_t);
    printf("header 占 %zu 字节, payload 起始地址 = %p, payload 剩余长度 = %zu\n",
           sizeof(uint32_t), (void*)payload, payloadLen);

    printf("\n注意：越界访问是未定义行为，例如 buffer + 100 已经越界，\n"
           "本示例不做这个操作，实际项目中务必自行做边界检查。\n");

}
