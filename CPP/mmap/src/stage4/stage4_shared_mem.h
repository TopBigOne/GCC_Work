/**
 * @file stage4_shared_mem.h
 * @brief 阶段 4：用 mmap 实现进程间共享内存（IPC）。
 *
 * 本阶段学习：
 * - fork + 匿名 MAP_SHARED：父子进程共享同一块物理内存
 * - POSIX 共享内存 shm_open + ftruncate + mmap：无亲缘关系的进程按名字找到同一块内存
 * - 共享内存本身不提供同步：用原子变量实现单生产者/单消费者（SPSC）无锁环形缓冲区
 *
 * 核心原理：MAP_SHARED 映射的页表项指向同一组物理页，一个进程写入，
 * 其他进程立刻能看到，数据不需要像管道/socket 那样在内核和用户态之间拷贝。
 *
 * @par 实际应用
 * - Chrome 多进程架构 ：渲染进程把绘制结果放进共享内存交给 GPU/浏览器进程，避免大块像素拷贝
 * - Android         ：ashmem / memfd + Binder 传 fd，SurfaceFlinger 与 App 共享图形缓冲区
 * - 数据库           ：PostgreSQL 的 shared_buffers 由多个后端进程共享
 * - 高频交易、行情分发 ：同机进程间用共享内存环形队列做微秒级低延迟通信
 * - 音视频           ：采集进程和编码进程之间共享帧缓冲区
 */
#pragma once

#include "../mmap_wrapper.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

namespace mmap_learn {

/**
 * @brief RAII 封装的 POSIX 共享内存：构造时 shm_open + ftruncate + mmap，析构时 munmap（+ shm_unlink）。
 *
 * 共享内存对象有一个以 '/' 开头的名字，任何知道名字且有权限的进程都能打开同一块内存。
 * 创建者（owner）负责在析构时 shm_unlink 删除这个名字。
 *
 * @code
 * // 进程 A
 * SharedMemory shm("/my_shm", 4096, true);
 * strcpy(shm.as<char>(), "hello");
 * // 进程 B
 * SharedMemory shm("/my_shm", 4096, false);
 * printf("%s\n", shm.as<char>()); // hello
 * @endcode
 *
 * @par 实际应用
 * 两个独立启动的程序（如行情接收服务和策略程序）之间传递大量数据。和 fork 共享不同，
 * 不要求进程之间有父子关系，只要约定好名字和数据结构布局即可。
 *
 * @note Linux 上 shm_open 实际是在 /dev/shm（tmpfs）下创建文件，可以用 ls /dev/shm 看到；
 *       macOS 没有 /dev/shm，名字长度上限约 31 字符（PSHMNAMLEN），
 *       且一个共享内存对象只能 ftruncate 一次。
 * @note shm_unlink 只是删除名字，已经映射的进程仍可继续使用，直到全部 munmap 后内存才真正释放，
 *       类似对文件执行 unlink。
 */
class SharedMemory {
    void *mapped_addr_ = MAP_FAILED; ///< mmap 实际返回的映射起始地址，无效时为 MAP_FAILED
    size_t size_ = 0;                ///< 映射长度（字节）
    std::string name_;               ///< 共享内存名字，以 '/' 开头
    bool owner_ = false;             ///< 是否为创建者，创建者析构时负责 shm_unlink

public:
    /**
     * @brief 创建或打开一块 POSIX 共享内存并映射到当前进程。
     * ------------------------------------------------------
     * @param name     共享内存名字，必须以 '/' 开头，如 "/mmap_learn_stage4"
     * @param size     映射长度（字节）；创建时会用 ftruncate 把共享内存对象设为该大小
     * @param create  true 表示创建（O_CREAT，成为 owner）；false 表示打开已存在的对象
     * @throw std::runtime_error shm_open、ftruncate 或 mmap 失败时抛出；创建过程中失败会先 shm_unlink 清理
     */
    SharedMemory(const char *name, size_t size, bool create);

    /** @brief munmap 当前进程的映射；若为创建者，同时 shm_unlink 删除名字。 */
    ~SharedMemory();

    /** @brief 禁止拷贝构造，避免重复 munmap / shm_unlink。 */
    SharedMemory(const SharedMemory &) = delete;

    /** @brief 禁止拷贝赋值，避免重复 munmap / shm_unlink。 */
    SharedMemory &operator=(const SharedMemory &) = delete;

    /**
     * @brief 移动构造，转移映射和 owner 身份。
     * @param other 源对象，移动后不再持有映射，也不再负责 shm_unlink
     */
    SharedMemory(SharedMemory &&other) noexcept;

    /**
     * @brief 移动赋值，先释放自身资源，再接管 other 的映射和 owner 身份。
     * @param other 源对象，移动后不再持有映射，也不再负责 shm_unlink
     * @return 自身引用
     */
    SharedMemory &operator=(SharedMemory &&other) noexcept;

    /**
     * @brief 获取共享内存起始地址（可写版本）。
     * @return 共享内存在当前进程中的起始地址，无效时为 MAP_FAILED
     */
    void *data();

    /**
     * @brief 获取共享内存起始地址（只读版本），供 const 对象使用。
     * @return 共享内存在当前进程中的起始地址，无效时为 MAP_FAILED
     */
    const void *data() const;

    /**
     * @brief 获取映射长度。
     * @return 映射长度（字节）
     */
    size_t size() const;

    /**
     * @brief 将共享内存按 T 类型指针访问（可写版本）。
     * @tparam T 目标类型；跨进程共享时应使用不含指针的平凡类型（各进程映射地址可能不同）
     * @return 指向起始地址的 T 指针
     */
    template <typename T>
    T *as() {
        return static_cast<T *>(mapped_addr_);
    }

    /**
     * @brief 将共享内存按 T 类型指针访问（只读版本），供 const 对象使用。
     * @tparam T 目标类型；跨进程共享时应使用不含指针的平凡类型
     * @return 指向起始地址的 const T 指针
     */
    template <typename T>
    const T *as() const {
        return static_cast<const T *>(mapped_addr_);
    }
};

/**
 * @brief 放在共享内存中的单生产者/单消费者（SPSC）无锁环形缓冲区。
 *
 * head_ 只由生产者写，tail_ 只由消费者写，双方通过 acquire/release 内存序同步，
 * 不需要互斥锁。始终空出一个槽位用来区分"满"和"空"，所以最多能存 N - 1 个元素。
 *
 * @tparam T 元素类型，必须可平凡拷贝（直接按字节复制，不含跨进程无效的指针）
 * @tparam N 槽位数量，实际容量为 N - 1
 *
 * @par 实际应用
 * 日志采集（业务进程写、日志进程读）、行情分发、音频数据传递等"一个写、一个读"的高吞吐场景。
 * 与管道/socket 相比，没有系统调用和数据拷贝，延迟可以做到亚微秒级。
 *
 * @note 原子变量能跨进程使用的前提是它是无锁的（std::atomic<uint32_t> 在主流平台上都是），
 *       有锁实现的锁在进程私有内存中，跨进程无效。
 * @note 如果需要多生产者或多消费者，就要改用 pthread_mutex 并设置 PTHREAD_PROCESS_SHARED 属性，
 *       或使用 CAS 实现的 MPMC 队列。
 * @warning 结构体中不能存放指针：同一块共享内存在不同进程中的映射地址可能不同。
 */
template <typename T, uint32_t N>
struct RingBuffer {
    static_assert(std::is_trivially_copyable<T>::value, "RingBuffer<T> requires trivially copyable T");
    static_assert(N > 0, "RingBuffer capacity must be > 0");

    std::atomic<uint32_t> head_{0}; ///< 下一个写入位置，只由生产者修改
    std::atomic<uint32_t> tail_{0}; ///< 下一个读取位置，只由消费者修改
    T data_[N];                     ///< 元素存储区

    /**
     * @brief 生产者写入一个元素。
     *
     * 先写数据，再用 release 发布新的 head_，保证消费者看到 head_ 更新时数据已经写好。
     *
     * @param item 要写入的元素
     * @return 成功返回 true；队列已满返回 false
     */
    bool push(const T &item) {
        uint32_t h = head_.load(std::memory_order_relaxed);
        uint32_t next = (h + 1) % N;
        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // 已满
        }
        data_[h] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    /**
     * @brief 消费者取出一个元素。
     *
     * 用 acquire 读取 head_，保证能看到生产者在 release 之前写入的数据。
     *
     * @param[out] item 取出的元素
     * @return 成功返回 true；队列为空返回 false
     */
    bool pop(T &item) {
        uint32_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire)) {
            return false; // 为空
        }
        item = data_[t];
        tail_.store((t + 1) % N, std::memory_order_release);
        return true;
    }

    /**
     * @brief 判断队列是否为空。
     * @return 为空时返回 true（并发下只是瞬时快照）
     */
    bool empty() const { return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire); }

    /**
     * @brief 判断队列是否已满。
     * @return 已满时返回 true（并发下只是瞬时快照）
     */
    bool full() const {
        uint32_t next = (head_.load(std::memory_order_acquire) + 1) % N;
        return next == tail_.load(std::memory_order_acquire);
    }

    /**
     * @brief 获取当前元素个数。
     * @return 元素个数（并发下只是瞬时快照）
     */
    uint32_t count() const {
        uint32_t h = head_.load(std::memory_order_acquire);
        uint32_t t = tail_.load(std::memory_order_acquire);
        return (h >= t) ? (h - t) : (N - t + h);
    }
};

/**
 * @brief 阶段 4 的演示入口：fork 共享、shm_open 共享、共享内存环形队列。
 */
class Stage4SharedMem {
public:
    /**
     * @brief Demo 1：MAP_SHARED | MAP_ANON 匿名共享映射，fork 后父子进程共享。
     *
     * 子进程写入字符串后退出，父进程等待后读到同样的内容。
     *
     * @par 实际应用
     * 多进程服务器（如 Nginx、早期 Apache prefork）在 master 进程中先建立共享映射，
     * 再 fork 出 worker，用来共享统计计数、限流状态、连接表等。
     *
     * @note 关键是 MAP_SHARED：如果换成 MAP_PRIVATE，fork 后子进程写入会触发写时复制（COW），
     *       父进程读到的仍是空字符串。
     */
    static void demo_fork_shared();

    /**
     * @brief Demo 2：通过 shm_open 按名字打开同一块共享内存。
     *
     * 父进程创建 "/mmap_learn_stage4"，子进程用同一个名字重新 shm_open 打开并写入，父进程读取。
     *
     * @par 实际应用
     * 这里为了演示方便用了 fork，但子进程并没有依赖继承来的映射，而是按名字重新打开，
     * 所以把子进程换成一个完全独立启动的程序也同样可行，这正是 shm_open 相比匿名共享映射的优势。
     */
    static void demo_shm_open();

    /**
     * @brief Demo 3：在共享内存中放置 RingBuffer，子进程生产、父进程消费。
     *
     * 用 placement new 直接在共享内存上构造 RingBuffer 对象，fork 后两个进程操作的是同一个队列。
     *
     * @par 实际应用
     * 这就是跨进程消息队列的雏形：把"数据结构"直接放进共享内存，双方按同样的类型解读，
     * 不需要序列化，也不需要系统调用。
     *
     * @note 父子进程并发收发：子进程 push，父进程同时 pop，收齐全部条目后才 waitpid。
     *       因此写入条目数可以超过队列容量（N-1 = 15）；队列满时生产者等待，消费者取走后继续。
     *       如果改成"先 waitpid 再 pop"，队列一满就会出现双方互相等待的死锁。
     */
    static void demo_ring_buffer();

    /** @brief 依次运行本阶段全部 demo。 */
    static void run_all();
};

} // namespace mmap_learn
