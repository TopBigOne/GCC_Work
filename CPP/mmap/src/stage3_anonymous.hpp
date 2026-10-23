#pragma once

#include "mmap_wrapper.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <new>
#include <unistd.h>

namespace mmap_learn {

// ============================================================
// Sample struct for arena demo
// ============================================================
struct Point {
    double x, y, z;
};

// ============================================================
// ArenaAllocator - bump allocator backed by anonymous mmap
// ============================================================
class ArenaAllocator {
    MmapRegion region_;
    size_t capacity_ = 0;
    size_t offset_ = 0;

    static size_t alignUp(size_t value, size_t align) { return (value + align - 1) & ~(align - 1); }

public:
    explicit ArenaAllocator(size_t initialCapacity = 1024 * 1024)
        : region_(MmapBuilder()
                      .length(initialCapacity)
                      .readWrite()
                      .priv()
                      .anonymous()
                      .build()),
          capacity_(initialCapacity),
          offset_(0) {}

    ~ArenaAllocator() = default;

    // Non-copyable, movable
    ArenaAllocator(const ArenaAllocator &) = delete;
    ArenaAllocator &operator=(const ArenaAllocator &) = delete;
    ArenaAllocator(ArenaAllocator &&) = default;
    ArenaAllocator &operator=(ArenaAllocator &&) = default;

    void *alloc(size_t n, size_t align = 8) {
        size_t aligned = alignUp(offset_, align);
        if (aligned + n > capacity_) {
            std::printf("[ArenaAllocator] out of memory: requested %zu, remaining %zu\n", n, capacity_ - aligned);
            return nullptr;
        }
        void *ptr = static_cast<uint8_t *>(region_.data()) + aligned;
        offset_ = aligned + n;
        return ptr;
    }

    template <typename T, typename... Args>
    T *create(Args &&...args) {
        void *ptr = alloc(sizeof(T), alignof(T));
        if (!ptr) return nullptr;
        return new (ptr) T{std::forward<Args>(args)...};
    }

    void reset() { offset_ = 0; }

    size_t used() const { return offset_; }
    size_t capacity() const { return capacity_; }
    size_t remaining() const { return capacity_ - offset_; }
};

// ============================================================
// Stage3Anonymous - demo driver
// ============================================================
class Stage3Anonymous {
public:
    // ----------------------------------------------------------
    // Demo 1: anonymous mapping basics
    // ----------------------------------------------------------
    static void demo_anonymous_map() {
        std::printf("\n========== Stage3: Anonymous Mapping ==========\n");

        constexpr size_t kSize = 4096;
        MmapRegion region = MmapBuilder()
                                .length(kSize)
                                .readWrite()
                                .priv()
                                .anonymous()
                                .build();

        std::printf("[anonymous] mapped %zu bytes at %p\n", region.size(), region.data());

        // Verify zero-initialization (anonymous pages are guaranteed zero)
        const uint8_t *bytes = region.as<uint8_t>();
        bool allZero = true;
        for (size_t i = 0; i < kSize; ++i) {
            if (bytes[i] != 0) {
                allZero = false;
                break;
            }
        }
        std::printf("[anonymous] zero-initialized: %s\n", allZero ? "YES" : "NO");

        // Write a message and read it back
        const char *msg = "Hello from anonymous mmap!";
        std::memcpy(region.data(), msg, std::strlen(msg) + 1);
        std::printf("[anonymous] wrote:  \"%s\"\n", region.as<char>());

        // Write integers at an offset
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

    // ----------------------------------------------------------
    // Demo 2: arena allocator
    // ----------------------------------------------------------
    static void demo_arena_allocator() {
        std::printf("\n========== Stage3: Arena Allocator ==========\n");

        ArenaAllocator arena(4096);
        std::printf("[arena] capacity: %zu, used: %zu, remaining: %zu\n", arena.capacity(), arena.used(),
                    arena.remaining());

        // Allocate some integers
        int *a = arena.create<int>(42);
        int *b = arena.create<int>(99);
        std::printf("[arena] int *a = %d (at %p)\n", *a, static_cast<void *>(a));
        std::printf("[arena] int *b = %d (at %p)\n", *b, static_cast<void *>(b));

        // Allocate a Point struct
        Point *p1 = arena.create<Point>(1.0, 2.0, 3.0);
        Point *p2 = arena.create<Point>(4.0, 5.0, 6.0);
        std::printf("[arena] Point p1 = (%.1f, %.1f, %.1f) at %p\n", p1->x, p1->y, p1->z, static_cast<void *>(p1));
        std::printf("[arena] Point p2 = (%.1f, %.1f, %.1f) at %p\n", p2->x, p2->y, p2->z, static_cast<void *>(p2));

        // Allocate a raw array of doubles
        constexpr size_t kArrLen = 10;
        double *arr = static_cast<double *>(arena.alloc(sizeof(double) * kArrLen, alignof(double)));
        for (size_t i = 0; i < kArrLen; ++i) {
            arr[i] = static_cast<double>(i) * 1.1;
        }
        std::printf("[arena] double array[0..4]: %.1f %.1f %.1f %.1f %.1f\n", arr[0], arr[1], arr[2], arr[3], arr[4]);

        std::printf("[arena] after allocations => used: %zu, remaining: %zu\n", arena.used(), arena.remaining());

        // Reset and reuse
        arena.reset();
        std::printf("[arena] after reset()     => used: %zu, remaining: %zu\n", arena.used(), arena.remaining());

        // Allocate again to show memory reuse
        int *c = arena.create<int>(777);
        std::printf("[arena] after reset, new int *c = %d (at %p)\n", *c, static_cast<void *>(c));
        std::printf("[arena] reuse confirmed: address %p is within the same arena region\n", static_cast<void *>(c));
    }

    // ----------------------------------------------------------
    // Demo 3: inspect process memory map
    // ----------------------------------------------------------
    static void demo_process_maps() {
        std::printf("\n========== Stage3: Process Memory Maps ==========\n");

        pid_t pid = ::getpid();

#ifdef __linux__
        // On Linux, read /proc/self/maps
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
        // On macOS, /proc doesn't exist. Print pid for manual vmmap inspection.
        std::printf("[maps] macOS detected. /proc/self/maps is not available.\n");
        std::printf("[maps] process PID = %d\n", pid);
        std::printf("[maps] to inspect memory regions, run in another terminal:\n");
        std::printf("[maps]   vmmap %d\n", pid);
        std::printf("[maps]   vmmap --summary %d\n", pid);

        // Also show a quick anonymous mmap to demonstrate it would appear in vmmap
        MmapRegion probe = MmapBuilder()
                               .length(65536)
                               .readWrite()
                               .priv()
                               .anonymous()
                               .build();
        std::printf("[maps] created a 64KB anonymous mapping at %p for inspection\n", probe.data());
        std::printf("[maps] this region would show up in vmmap output as MALLOC or unnamed\n");
#else
        std::printf("[maps] unsupported platform. PID = %d\n", pid);
#endif
    }

    // ----------------------------------------------------------
    // Run all demos
    // ----------------------------------------------------------
    static void run_all() {
        demo_anonymous_map();
        demo_arena_allocator();
        demo_process_maps();
    }
};

} // namespace mmap_learn
