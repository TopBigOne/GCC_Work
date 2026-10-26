/**
 * @file stage3_anonymous.h
 * @brief 阶段 3：匿名映射与基于 mmap 的 Arena 内存池。
 *
 * 本阶段学习：
 * - 匿名映射（MAP_ANONYMOUS | MAP_PRIVATE）：不关联任何文件，直接向内核要一块"纯内存"
 * - 零页与按需分配：刚映射时不占物理内存，第一次写某页时才触发缺页、分配物理页，且内容保证为 0
 * - 用匿名映射实现一个简单的 Arena（bump）分配器
 * - 观察进程地址空间：Linux 的 /proc/self/maps，macOS 的 vmmap
 *
 * @par 实际应用
 * - glibc malloc：超过 M_MMAP_THRESHOLD（默认约 128KB）的大块分配直接走匿名 mmap，
 *   free 时 munmap 立即归还给系统，避免堆碎片
 * - 内存池 / Arena：游戏引擎每帧临时对象、编译器 AST 节点、protobuf Arena、数据库查询执行期内存，
 *   一次性申请大块、快速顺序分配、整体释放
 * - 线程栈：pthread 创建线程时用匿名 mmap 分配栈空间，并在末尾放一页 PROT_NONE 作为保护页
 * - JIT / 虚拟机堆：V8、JVM 先用匿名 mmap 预留大段地址空间，再按需提交
 */
#pragma once

#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <new>
#include <utility>

namespace mmap_learn {

/**
 * @brief Arena 演示用的简单结构体，表示三维空间中的一个点。
 */
struct Point {
    double x; ///< x 坐标
    double y; ///< y 坐标
    double z; ///< z 坐标
};

/**
 * @brief 基于匿名 mmap 的 Arena（bump）分配器。
 *
 * 构造时用匿名映射申请一整块内存，之后每次分配只是把偏移量 offset_ 往后"推"一段（bump），
 * 不记录每块的大小、不支持单独释放，只能通过 reset() 一次性回收全部内存。
 *
 * @code
 * ArenaAllocator arena(4096);
 * int   *a = arena.create<int>(42);
 * Point *p = arena.create<Point>(1.0, 2.0, 3.0);
 * arena.reset(); // 全部回收，内存可重复使用
 * @endcode
 *
 * @par 实际应用
 * 适合"大量小对象、生命周期相同"的场景：一帧渲染、一次请求、一次编译过程中产生的临时对象。
 * 相比逐个 new/delete：分配只需一次加法，没有锁和空闲链表开销；释放是 O(1)；对象连续存放，缓存友好。
 * 底层用 mmap 而不是 malloc，可以直接控制页属性（mprotect、madvise(MADV_DONTNEED) 归还物理页），
 * 且不会和 malloc 的堆互相干扰。
 *
 * @note 匿名映射刚建立时不占物理内存，只有真正写到的页才会分配，
 *       所以预留一个很大的 Arena 并不会立刻消耗同等大小的 RSS。
 * @warning reset() 不会调用对象的析构函数，只适合存放平凡可析构（trivially destructible）的类型，
 *          或调用方自行负责析构。
 */
class ArenaAllocator {
    MmapRegion region_;   ///< 底层匿名映射，Arena 析构时自动 munmap
    size_t capacity_ = 0; ///< 总容量（字节）
    size_t offset_ = 0;   ///< 下一次分配的起始偏移，即已使用的字节数

    /**
     * @brief 把 value 向上对齐到 align 的整数倍。
     * @param value 待对齐的值
     * @param align 对齐粒度，必须是 2 的幂
     * @return 不小于 value 的最小的 align 倍数，如 alignUp(13, 8) == 16
     */
    static size_t alignUp(size_t value, size_t align);

public:
    /**
     * @brief 通过匿名私有映射申请一块可读写内存作为 Arena。
     * @param initialCapacity 容量（字节），默认 1MB；内核会向上取整到页大小
     * @throw std::runtime_error mmap 失败时抛出
     */
    explicit ArenaAllocator(size_t initialCapacity = 1024 * 1024);

    /** @brief 由 region_ 的析构函数 munmap 整块内存，所有对象随之失效。 */
    ~ArenaAllocator() = default;

    /** @brief 禁止拷贝构造，避免两个 Arena 共享同一块映射。 */
    ArenaAllocator(const ArenaAllocator &) = delete;

    /** @brief 禁止拷贝赋值，避免两个 Arena 共享同一块映射。 */
    ArenaAllocator &operator=(const ArenaAllocator &) = delete;

    /**
     * @brief 移动构造，转移底层映射的所有权。
     * @param other 源 Arena；移动后变为空 Arena（容量 0），对它调用 alloc() 返回 nullptr
     */
    ArenaAllocator(ArenaAllocator &&other) noexcept;

    /**
     * @brief 移动赋值，释放自身映射后接管另一个 Arena 的映射。
     * @param other 源 Arena；移动后变为空 Arena（容量 0），对它调用 alloc() 返回 nullptr
     * @return 自身引用
     */
    ArenaAllocator &operator=(ArenaAllocator &&other) noexcept;

    /**
     * @brief 从 Arena 中分配 n 字节原始内存。
     *
     * 先把当前偏移向上对齐到 align，再判断剩余空间是否足够，足够则把偏移推进 n 字节。
     * 剩余空间的判断写成 `n > capacity_ - aligned`，避免 `aligned + n` 在 n 很大时溢出回绕。
     *
     * @param n     需要的字节数
     * @param align 对齐要求，必须是 2 的幂，默认 8
     * @return 分配到的地址；空间不足或 Arena 已被移走（无效映射）时打印提示并返回 nullptr
     */
    void *alloc(size_t n, size_t align = 8);

    /**
     * @brief 在 Arena 中分配并构造一个 T 对象。
     *
     * 先按 sizeof(T)、alignof(T) 分配内存，再用 placement new 在该地址上构造对象。
     *
     * @tparam T    要创建的类型
     * @tparam Args 构造参数类型
     * @param args  转发给 T 的构造参数（使用花括号初始化，所以聚合类型如 Point 也可用）
     * @return 指向新对象的指针；空间不足时返回 nullptr
     */
    template <typename T, typename... Args>
    T *create(Args &&...args) {
        void *ptr = alloc(sizeof(T), alignof(T));
        if (!ptr) return nullptr;
        return new (ptr) T{std::forward<Args>(args)...};
    }

    /**
     * @brief 一次性回收全部内存：只把偏移归零，不 munmap、不调用析构函数。
     * @note 之前分配出去的指针全部失效，但物理页仍保留，下次分配可以直接复用。
     */
    void reset();

    /**
     * @brief 获取已使用的字节数。
     * @return 已使用的字节数（含对齐填充）
     */
    size_t used() const;

    /**
     * @brief 获取总容量。
     * @return 总容量（字节）
     */
    size_t capacity() const;

    /**
     * @brief 获取剩余可用字节数。
     * @return capacity() - used()
     */
    size_t remaining() const;
};

/**
 * @brief 阶段 3 的演示入口：匿名映射、Arena 分配器、进程内存布局观察。
 */
class Stage3Anonymous {
public:
    /**
     * @brief Demo 1：匿名映射的基本用法，验证内容初始为 0，并读写字符串和整数。
     *
     * @par 实际应用
     * 需要一大块临时内存（如图片解码缓冲区、大数组）又不想走 malloc 时，直接匿名 mmap。
     * 内核保证内容全 0，省去 memset；用完 munmap 立即归还系统，不会像 free 那样可能留在进程堆里。
     *
     * @note 零页机制：匿名私有映射在读之前指向一个共享的全零物理页，第一次写入时才触发缺页、
     *       分配真正的物理页（写时复制）。因此"映射了 4KB"不等于"占用了 4KB 物理内存"。
     */
    static void demo_anonymous_map();

    /**
     * @brief Demo 2：使用 ArenaAllocator 分配 int、Point、double 数组，然后 reset 并复用。
     *
     * @par 实际应用
     * 展示内存池的核心价值：连续分配、零碎片、整体释放。例如 HTTP 服务器为每个请求建一个 Arena，
     * 请求结束时 reset，下一个请求复用同一块内存，避免频繁 malloc/free 和系统调用。
     */
    static void demo_arena_allocator();

    /**
     * @brief Demo 3：查看当前进程的内存映射布局。
     *
     * Linux 上读取 /proc/self/maps 打印前 30 行；macOS 上先建立一个 64KB 的 probe 映射，
     * 在它仍然存活时通过 popen 运行 `vmmap <pid>`，打印包含 probe 地址的行；
     * vmmap 不可用时退化为提示用户手动执行。
     *
     * @par 实际应用
     * 排查内存问题的第一步：内存涨了是堆、mmap 区、还是某个 .so / 文件映射？
     * /proc/<pid>/maps 列出每段映射的地址范围、权限（rwxp/s）、偏移和对应文件，
     * 配合 /proc/<pid>/smaps 还能看到每段实际占用的 RSS、脏页数量。
     *
     * @note 平台差异：macOS 没有 /proc 文件系统，只能借助 `vmmap <pid>` 查看；
     *       Linux 上匿名映射在 maps 中显示为没有文件名的行。
     */
    static void demo_process_maps();

    /** @brief 依次运行本阶段全部 demo。 */
    static void run_all();
};

} // namespace mmap_learn
