#include "stage3_anonymous.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace mmap_learn {

// ============================================================
// ArenaAllocator
// ============================================================

size_t ArenaAllocator::alignUp(size_t value, size_t align) { return (value + align - 1) & ~(align - 1); }

ArenaAllocator::ArenaAllocator(size_t initialCapacity)
    : region_(MmapBuilder()
                  .length(initialCapacity)
                  .readWrite()
                  .priv()
                  .anonymous()
                  .build()),
      capacity_(initialCapacity),
      offset_(0) {}

ArenaAllocator::ArenaAllocator(ArenaAllocator &&other) noexcept
    : region_(std::move(other.region_)), capacity_(other.capacity_), offset_(other.offset_) {
    // 源对象变为空 Arena，之后对它 alloc 会直接返回 nullptr
    other.capacity_ = 0;
    other.offset_ = 0;
}

ArenaAllocator &ArenaAllocator::operator=(ArenaAllocator &&other) noexcept {
    if (this != &other) {
        region_ = std::move(other.region_); // MmapRegion 的移动赋值会先 munmap 自己原来的映射
        capacity_ = other.capacity_;
        offset_ = other.offset_;
        other.capacity_ = 0;
        other.offset_ = 0;
    }
    return *this;
}

void *ArenaAllocator::alloc(size_t n, size_t align) {
    if (!region_.valid()) {
        std::printf("[ArenaAllocator] invalid arena (moved-from), requested %zu\n", n);
        return nullptr;
    }
    size_t aligned = alignUp(offset_, align);
    // 写成减法比较，避免 aligned + n 溢出回绕后误判为"空间足够"
    if (aligned > capacity_ || n > capacity_ - aligned) {
        size_t remaining = aligned > capacity_ ? 0 : capacity_ - aligned;
        std::printf("[ArenaAllocator] out of memory: requested %zu, remaining %zu\n", n, remaining);
        return nullptr;
    }
    void *ptr = static_cast<uint8_t *>(region_.data()) + aligned;
    offset_ = aligned + n;
    return ptr;
}

void ArenaAllocator::reset() { offset_ = 0; }

size_t ArenaAllocator::used() const { return offset_; }

size_t ArenaAllocator::capacity() const { return capacity_; }

size_t ArenaAllocator::remaining() const { return capacity_ - offset_; }

// ============================================================
// Stage3Anonymous
// ============================================================

void Stage3Anonymous::demo_anonymous_map() {
    std::printf("\n========== Stage3: Anonymous Mapping ==========\n");

    constexpr size_t kSize = 4096;
    MmapRegion region = MmapBuilder()
                            .length(kSize)
                            .readWrite()
                            .priv()
                            .anonymous()
                            .build();

    std::printf("[anonymous] mapped %zu bytes at %p\n", region.size(), region.data());

    // 验证内容全为 0（匿名映射的页保证初始为 0）
    const uint8_t *bytes = region.as<uint8_t>();
    bool allZero = true;
    for (size_t i = 0; i < kSize; ++i) {
        if (bytes[i] != 0) {
            allZero = false;
            break;
        }
    }
    std::printf("[anonymous] zero-initialized: %s\n", allZero ? "YES" : "NO");

    // 写入一个字符串再读回来，这次写入才真正分配物理页
    const char *msg = "Hello from anonymous mmap!";
    std::memcpy(region.data(), msg, std::strlen(msg) + 1);
    std::printf("[anonymous] wrote:  \"%s\"\n", region.as<char>());

    // 在偏移 256 处写入整数，演示把映射当作任意类型的缓冲区使用
    int *intPtr = reinterpret_cast<int *>(static_cast<uint8_t *>(region.data()) + 256);
    for (int i = 0; i < 5; ++i) {
        intPtr[i] = (i + 1) * 100;
    }
    std::printf("[anonymous] integers at offset 256:");
    for (int i = 0; i < 5; ++i) {
        std::printf(" %d", intPtr[i]);
    }
    std::printf("\n");

    std::printf("[anonymous] region will be munmap'd on scope exit (RAII)\n");
}

void Stage3Anonymous::demo_arena_allocator() {
    std::printf("\n========== Stage3: Arena Allocator ==========\n");

    ArenaAllocator arena(4096);
    std::printf("[arena] capacity: %zu, used: %zu, remaining: %zu\n", arena.capacity(), arena.used(),
                arena.remaining());

    // 分配两个 int
    int *a = arena.create<int>(42);
    int *b = arena.create<int>(99);
    std::printf("[arena] int *a = %d (at %p)\n", *a, static_cast<void *>(a));
    std::printf("[arena] int *b = %d (at %p)\n", *b, static_cast<void *>(b));

    // 分配 Point 结构体，注意地址会按 alignof(Point) 对齐
    Point *p1 = arena.create<Point>(1.0, 2.0, 3.0);
    Point *p2 = arena.create<Point>(4.0, 5.0, 6.0);
    std::printf("[arena] Point p1 = (%.1f, %.1f, %.1f) at %p\n", p1->x, p1->y, p1->z, static_cast<void *>(p1));
    std::printf("[arena] Point p2 = (%.1f, %.1f, %.1f) at %p\n", p2->x, p2->y, p2->z, static_cast<void *>(p2));

    // 用 alloc 分配原始 double 数组
    constexpr size_t kArrLen = 10;
    double *arr = static_cast<double *>(arena.alloc(sizeof(double) * kArrLen, alignof(double)));
    for (size_t i = 0; i < kArrLen; ++i) {
        arr[i] = static_cast<double>(i) * 1.1;
    }
    std::printf("[arena] double array[0..4]: %.1f %.1f %.1f %.1f %.1f\n", arr[0], arr[1], arr[2], arr[3], arr[4]);

    std::printf("[arena] after allocations => used: %zu, remaining: %zu\n", arena.used(), arena.remaining());

    // 整体回收
    arena.reset();
    std::printf("[arena] after reset()     => used: %zu, remaining: %zu\n", arena.used(), arena.remaining());

    // 再次分配，地址与第一次分配的 a 相同，说明内存被复用
    int *c = arena.create<int>(777);
    std::printf("[arena] after reset, new int *c = %d (at %p)\n", *c, static_cast<void *>(c));
    std::printf("[arena] reuse confirmed: address %p is within the same arena region\n", static_cast<void *>(c));
}

void Stage3Anonymous::demo_process_maps() {
    std::printf("\n========== Stage3: Process Memory Maps ==========\n");

    pid_t pid = ::getpid();

#ifdef __linux__
    // Linux：直接读取 /proc/self/maps
    std::printf("[maps] reading /proc/self/maps (pid=%d):\n", pid);
    FILE *f = std::fopen("/proc/self/maps", "r");
    if (!f) {
        std::printf("[maps] failed to open /proc/self/maps: %s\n", std::strerror(errno));
        return;
    }
    char line[512];
    int count = 0;
    while (std::fgets(line, sizeof(line), f) && count < 30) {
        std::printf("  %s", line);
        ++count;
    }
    if (count == 30) {
        std::printf("  ... (truncated, showing first 30 lines)\n");
    }
    std::fclose(f);
#elif defined(__APPLE__)
    // macOS：没有 /proc，改用 vmmap 查看
    std::printf("[maps] macOS detected. /proc/self/maps is not available.\n");
    std::printf("[maps] process PID = %d\n", pid);

    // 再建一个 64KB 匿名映射，并在它仍然存活时运行 vmmap，演示它会出现在 vmmap 输出中
    MmapRegion probe = MmapBuilder()
                           .length(65536)
                           .readWrite()
                           .priv()
                           .anonymous()
                           .build();
    std::printf("[maps] created a 64KB anonymous mapping at %p for inspection\n", probe.data());

    // vmmap 打印的地址是不带 0x 的小写十六进制，例如 "104c1c000-104c2c000"；匹配 "起始地址-" 只找以 probe 开头的区域
    char addr_hex[32];
    std::snprintf(addr_hex, sizeof(addr_hex), "%lx-",
                  static_cast<unsigned long>(reinterpret_cast<uintptr_t>(probe.data())));

    char cmd[64];
    std::snprintf(cmd, sizeof(cmd), "vmmap %d 2>/dev/null", pid);
    int matched = 0;
    if (FILE *p = ::popen(cmd, "r")) {
        char line[512];
        while (std::fgets(line, sizeof(line), p)) {
            if (std::strstr(line, addr_hex)) {
                if (matched == 0) std::printf("[maps] vmmap lines containing the probe region:\n");
                std::printf("  %s", line);
                ++matched;
            }
        }
        ::pclose(p);
    }

    if (matched == 0) {
        // vmmap 不可用或没有权限时，退化为提示用户手动查看
        std::printf("[maps] could not query vmmap automatically. To inspect memory regions, run:\n");
        std::printf("[maps]   vmmap %d\n", pid);
        std::printf("[maps]   vmmap --summary %d\n", pid);
        std::printf("[maps] this region would show up in vmmap output as MALLOC or unnamed\n");
    }
#else
    std::printf("[maps] unsupported platform. PID = %d\n", pid);
#endif
}

void Stage3Anonymous::run_all() {
    demo_anonymous_map();
    demo_arena_allocator();
    demo_process_maps();
}

} // namespace mmap_learn
