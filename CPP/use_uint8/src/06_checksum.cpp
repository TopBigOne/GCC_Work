// 06_checksum.cpp
// 演示：
//   1. 用 uint8_t 数组实现一个简单的 ByteBuffer（序列化容器）
//   2. 计算校验和（checksum），常用于校验数据完整性
//
// 编译：g++ -std=c++17 06_checksum.cpp -o 06_checksum

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <memory>
#include <stdexcept>

// ---------- 一个简单的字节缓冲区，支持写入不同类型的数据 ----------
class ByteBuffer {
public:
    explicit ByteBuffer(size_t capacity)
        : data_(std::make_unique<uint8_t[]>(capacity)), capacity_(capacity), size_(0) {}

    // 写入任意 POD 类型（内部按字节 memcpy）
    template <typename T>
    void write(const T& value) {
        if (size_ + sizeof(T) > capacity_) {
            throw std::overflow_error("ByteBuffer 容量不足");
        }
        std::memcpy(data_.get() + size_, &value, sizeof(T));
        size_ += sizeof(T);
    }

    // 写入原始字节数组（比如字符串）
    void writeBytes(const uint8_t* bytes, size_t len) {
        if (size_ + len > capacity_) {
            throw std::overflow_error("ByteBuffer 容量不足");
        }
        std::memcpy(data_.get() + size_, bytes, len);
        size_ += len;
    }

    const uint8_t* data() const { return data_.get(); }
    size_t size() const { return size_; }

private:
    std::unique_ptr<uint8_t[]> data_;
    size_t capacity_;
    size_t size_;
};

// ---------- 简单校验和算法：逐字节累加 + 循环左移（类似简化版 checksum） ----------
uint8_t computeChecksum(const uint8_t* data, size_t len) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < len; ++i) {
        checksum += data[i];                       // 累加
        checksum = (checksum << 1) | (checksum >> 7);  // 循环左移 1 位（8位范围内）
    }
    return checksum;
}

void demo_06_checksum() {
    printf("========== 1. 用 ByteBuffer 序列化一组混合类型数据 ==========\n");

    ByteBuffer buf(64);
    uint32_t userId = 10086;
    double   score  = 99.5;
    const char* name = "uint8_t";

    buf.write(userId);
    buf.write(score);
    buf.writeBytes(reinterpret_cast<const uint8_t*>(name), std::strlen(name));

    printf("序列化后总字节数 = %zu\n", buf.size());
    printf("原始字节（十六进制）: ");
    for (size_t i = 0; i < buf.size(); ++i) {
        printf("%02x ", buf.data()[i]);
    }
    printf("\n");

    printf("\n========== 2. 计算校验和，验证数据完整性 ==========\n");
    uint8_t sum1 = computeChecksum(buf.data(), buf.size());
    printf("原始数据校验和 = 0x%02x\n", sum1);

    printf("\n========== 3. 模拟数据在传输中被篡改一个字节，校验和随之变化 ==========\n");
    std::vector<uint8_t> tampered(buf.data(), buf.data() + buf.size());
    tampered[0] ^= 0xFF;   // 人为篡改第一个字节
    uint8_t sum2 = computeChecksum(tampered.data(), tampered.size());
    printf("篡改后数据校验和 = 0x%02x\n", sum2);

    if (sum1 != sum2) {
        printf("校验和不匹配 —— 成功检测出数据被篡改！\n");
    }

}
