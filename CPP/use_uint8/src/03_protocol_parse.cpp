// 03_protocol_parse.cpp
// 演示：用 uint8_t* 解析一段自定义的二进制协议（模拟网络包/文件格式场景）
//
// 协议格式设计（小端序 little-endian）：
// [0]      : uint8_t   version       版本号
// [1]      : uint8_t   msgType       消息类型
// [2..3]   : uint16_t  payloadLen    payload 长度
// [4..N-1] : uint8_t[] payload       实际数据
//
// 编译：g++ -std=c++17 03_protocol_parse.cpp -o 03_protocol_parse

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

#pragma pack(push, 1)
struct PacketHeader {
    uint8_t  version;
    uint8_t  msgType;
    uint16_t payloadLen;
};
#pragma pack(pop)

// 构造一个二进制包（序列化）
std::vector<uint8_t> buildPacket(uint8_t version, uint8_t msgType, const std::string& payload) {
    std::vector<uint8_t> packet;

    PacketHeader header;
    header.version = version;
    header.msgType = msgType;
    header.payloadLen = static_cast<uint16_t>(payload.size());

    // 先放 header 的原始字节
    packet.resize(sizeof(PacketHeader));
    std::memcpy(packet.data(), &header, sizeof(PacketHeader));

    // 再追加 payload 的字节
    packet.insert(packet.end(), payload.begin(), payload.end());

    return packet;
}

// 解析二进制包（反序列化）
void parsePacket(const uint8_t* data, size_t len) {
    if (len < sizeof(PacketHeader)) {
        printf("包长度不足，无法解析 header\n");
        return;
    }

    PacketHeader header;
    std::memcpy(&header, data, sizeof(PacketHeader));  // 安全读取，不用担心对齐问题

    printf("解析结果:\n");
    printf("  version     = %u\n", header.version);
    printf("  msgType     = %u\n", header.msgType);
    printf("  payloadLen  = %u\n", header.payloadLen);

    const uint8_t* payloadPtr = data + sizeof(PacketHeader);   // 按字节偏移，跳过 header
    size_t availablePayload = len - sizeof(PacketHeader);

    if (header.payloadLen > availablePayload) {
        printf("  警告：声明的 payloadLen 超出实际可用字节数，数据可能被截断或损坏\n");
        return;
    }

    std::string payload(reinterpret_cast<const char*>(payloadPtr), header.payloadLen);
    printf("  payload     = \"%s\"\n", payload.c_str());
}

void demo_03_protocol_parse() {
    printf("========== 1. 构造一个二进制包 ==========\n");
    std::vector<uint8_t> packet = buildPacket(1, 42, "hello uint8_t");

    printf("包的原始字节（十六进制）：\n");
    for (uint8_t b : packet) {
        printf("%02x ", b);
    }
    printf("\n总长度 = %zu 字节\n", packet.size());

    printf("\n========== 2. 从原始字节流反向解析出结构化数据 ==========\n");
    parsePacket(packet.data(), packet.size());

    printf("\n========== 3. 模拟数据被截断的异常情况 ==========\n");
    parsePacket(packet.data(), sizeof(PacketHeader) + 3);  // 只给一部分 payload

}
