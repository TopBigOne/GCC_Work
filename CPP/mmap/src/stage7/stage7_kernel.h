/**
 * @file stage7_kernel.h
 * @brief 阶段 7：内核原理观测 —— 看清 mmap 调用之后内核里发生了什么。
 *
 * 本阶段从三个角度理解 mmap 的内核实现：
 * 1. 事件观测    ：用观察者模式记录 mmap / mprotect / msync / mlock / munmap，
 *                 建立"每个系统调用都在修改 VMA 或页表"的直觉
 * 2. 地址空间观测 ：读取 /proc/self/maps，亲眼看到 mmap 出来的区域出现在进程地址空间里
 * 3. 原理讲解     ：VMA（vm_area_struct）、do_mmap 调用路径、缺页处理
 *
 * 对应知识点：
 * - VMA 与 mm_struct            ：每次 mmap 至少对应一个 VMA；mprotect 改变部分区域的权限会把 VMA 拆分
 * - 按需分页（demand paging）     ：mmap 只登记 VMA，不分配物理页；首次访问时触发缺页才分配
 * - 缺页的几种处理                ：匿名页清零、文件页从 page cache 读入、写时复制（COW）、从 swap 换入
 * - 透明大页（THP）/ MAP_HUGETLB  ：用 2MB 大页减少 TLB miss
 * - 安全                        ：ASLR 让每次 mmap 的地址随机；W^X 要求同一页不能同时可写和可执行，
 *                                JIT 编译器需要先写代码再 mprotect 成可执行
 *
 * @par 实际应用
 * - 性能排查            ：进程 RSS 远小于 VSZ 是正常的（按需分页）；大量 minor fault 意味着在频繁首次触碰新页
 * - 内存泄漏/碎片定位    ：通过 /proc/<pid>/maps、smaps 查看是谁占了地址空间，映射数是否超过 vm.max_map_count
 * - 调试崩溃            ：SIGSEGV 的地址落在哪个 VMA、权限是什么，可以判断是越界、写只读页还是访问已 munmap 的区域
 * - JIT / 动态代码生成（V8、LuaJIT、Java HotSpot）：依赖 mmap + mprotect 切换可写 / 可执行
 *
 * @note /proc/self/maps 是 Linux 专有的；macOS 上没有，需要用 `vmmap <pid>` 命令查看。
 *       VMA、do_mmap 等内核结构也都是 Linux 内核中的概念，XNU 内核的实现不同（vm_map_entry）。
 */
#pragma once

#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <cstdio>
#include <string>
#include <sys/mman.h>
#include <sys/types.h>
#include <vector>

namespace mmap_learn {

/**
 * @brief 映射相关操作的事件类型，每种对应一个系统调用。
 */
enum class MemoryEventType {
    MAPPED,    ///< mmap    ：新建 VMA
    UNMAPPED,  ///< munmap  ：删除 VMA 及其页表项
    PROTECTED, ///< mprotect：修改 VMA 权限（可能拆分 VMA）
    SYNCED,    ///< msync   ：把脏页写回文件
    LOCKED     ///< mlock   ：把页锁在物理内存中
};

/**
 * @brief 一次映射操作的描述，由 ObservableMmapRegion 发给观察者。
 */
struct MemoryEvent {
    MemoryEventType event_type; ///< 事件类型
    void *address;              ///< 映射起始地址
    size_t size;                ///< 映射长度（字节）
    std::string description;    ///< 可读描述，包含参数和成功 / 失败
};

/**
 * @brief 内存事件观察者接口（观察者模式）。
 *
 * @par 实际应用
 * 真实项目中可以实现不同的观察者：打印日志、统计各类操作次数、
 * 记录映射总量用于内存监控、在 mprotect 变成可执行时做安全审计等，
 * 而映射代码本身不需要改动。
 */
class IMemoryObserver {
public:
    /** @brief 虚析构函数，保证通过接口指针删除实现类对象时行为正确。 */
    virtual ~IMemoryObserver() = default;

    /**
     * @brief 收到一次映射事件时被调用。
     * @param event 事件详情
     */
    virtual void on_event(const MemoryEvent &event) = 0;
};

/**
 * @brief 把事件按固定格式打印到 stdout 的观察者。
 */
class ConsoleObserver : public IMemoryObserver {
    /**
     * @brief 事件类型转字符串。
     * @param t 事件类型
     * @return 类型名称，如 "MAPPED"
     */
    static const char *event_type_str(MemoryEventType t);

public:
    /**
     * @brief 打印一行事件：类型 | 地址 | 大小 | 描述。
     * @param event 事件详情
     */
    void on_event(const MemoryEvent &event) override;
};

/**
 * @brief 带事件通知的映射区域：包装 MmapRegion，每次操作后通知所有观察者。
 *
 * 只有经过本类方法的系统调用才会产生事件；直接通过 data() 读写内存
 * 不会通知（那只是普通的内存访问，内核只在缺页时才会介入）。
 *
 * @par 实际应用
 * 给映射操作加"埋点"：调试时追踪映射的生命周期，排查"谁在什么时候
 * 改了权限 / 解除了映射"，或统计程序运行中的 mmap 次数和总量。
 *
 * @warning 观察者以裸指针保存，不持有所有权；调用方需保证观察者
 *          的生命周期长于本对象，或者在销毁前调用 remove_observer()。
 */
class ObservableMmapRegion {
    MmapRegion region_;                        ///< 实际持有的映射（RAII）
    std::vector<IMemoryObserver *> observers_; ///< 已注册的观察者，不持有所有权

    /**
     * @brief 构造事件并依次通知所有观察者。
     * @param type 事件类型
     * @param desc 事件描述
     */
    void notify(MemoryEventType type, const std::string &desc);

public:
    /** @brief 构造一个尚未映射的对象，之后调用 map() 建立映射。 */
    ObservableMmapRegion() = default;

    /**
     * @brief 注册观察者。
     * @param obs 观察者指针，不转移所有权
     */
    void add_observer(IMemoryObserver *obs);

    /**
     * @brief 注销观察者（erase-remove 惯用法）。
     * @param obs 要移除的观察者指针
     */
    void remove_observer(IMemoryObserver *obs);

    /**
     * @brief 建立映射并发出 MAPPED 事件；若已有映射，旧映射会被自动 munmap。
     * @param length 映射长度
     * @param prot   访问权限 PROT_*
     * @param flags  映射标志 MAP_*
     * @param fd     文件描述符，匿名映射传 -1
     * @param offset 文件偏移，须按页对齐
     * @throw std::runtime_error mmap 失败时抛出（此时不会发出事件）
     * @note 内核此时只是新建了一个 VMA，还没有分配任何物理页。
     */
    void map(size_t length, int prot, int flags, int fd = -1, off_t offset = 0);

    /**
     * @brief 发出 UNMAPPED 事件后解除映射。
     *
     * 先通知再释放，这样观察者收到的事件里地址和大小仍然有效。
     */
    void unmap();

    /**
     * @brief 修改映射权限并发出 PROTECTED 事件。
     * @param prot 新权限 PROT_* 组合
     * @return 成功返回 0，失败返回 -1
     * @note 改成 PROT_READ 后再写会触发 SIGSEGV。JIT 编译器就是先以 RW 写入
     *       机器码，再 mprotect 成 RX 执行，以满足 W^X（写与执行互斥）。
     */
    int protect(int prot);

    /**
     * @brief 把映射中的修改写回文件，并发出 SYNCED 事件。
     * @param flags MS_SYNC 同步等待，MS_ASYNC 异步发起
     * @return 成功返回 0，失败返回 -1
     * @note 只对 MAP_SHARED 的文件映射有实际意义；对匿名映射调用通常直接成功、什么也不做。
     */
    int sync(int flags = MS_SYNC);

    /**
     * @brief 把映射的页锁在物理内存中，并发出 LOCKED 事件。
     * @return 成功返回 0，失败返回 -1（常见原因：超过 RLIMIT_MEMLOCK）
     * @note mlock 会立即对所有页触发缺页、分配物理内存，打破按需分页。
     *       常用于存放密钥（防止被换出到 swap）和低延迟场景（避免运行时缺页）。
     */
    int lock();

    /**
     * @brief 获取映射起始地址（可写版本）。直接读写内存不会产生事件。
     * @return 映射起始地址，未映射时为 MAP_FAILED
     */
    void *data();

    /**
     * @brief 获取映射起始地址（只读版本），供 const 对象使用。
     * @return 映射起始地址的只读指针
     */
    const void *data() const;

    /**
     * @brief 获取映射长度。
     * @return 映射长度（字节）
     */
    size_t size() const;

    /**
     * @brief 是否持有有效映射。
     * @return 持有时返回 true
     */
    bool valid() const;

    /**
     * @brief 把映射区域当作 T 类型指针使用。
     * @tparam T 目标类型
     * @return 指向映射起始地址的 T 指针
     */
    template <typename T>
    T *as() {
        return region_.as<T>();
    }
};

/**
 * @brief /proc/self/maps 中的一行，对应内核中的一个 VMA。
 *
 * 示例行：
 * @code
 * 7f3a2c000000-7f3a2c001000 rw-p 00000000 00:00 0
 * 7f3a2d200000-7f3a2d3c5000 r-xp 00000000 08:01 1835 /usr/lib/libc.so.6
 * @endcode
 */
struct ProcessMapEntry {
    unsigned long addr_start; ///< VMA 起始虚拟地址（vm_start）
    unsigned long addr_end;   ///< VMA 结束虚拟地址，不含（vm_end）
    char perms[5];            ///< 权限，如 "rw-p"：r/w/x + p(私有, MAP_PRIVATE) 或 s(共享, MAP_SHARED)
    unsigned long offset;     ///< 文件内偏移（字节），匿名映射为 0
    char device[12];          ///< 文件所在设备号 "主:次"，如 "08:01"；匿名映射为 "00:00"
    unsigned long inode;      ///< 文件 inode 号，匿名映射为 0
    std::string pathname;     ///< 映射的文件路径，或 [heap]、[stack]、[vdso] 等；匿名映射为空
};

/**
 * @brief 读取并解析当前进程的内存映射表（/proc/self/maps）。
 *
 * @par 实际应用
 * - 排查内存问题：看进程有多少个映射、哪些库被加载、堆和栈在哪里
 * - 定位崩溃：拿 SIGSEGV 的出错地址查它属于哪个 VMA、该 VMA 是什么权限
 * - 安全分析：检查是否存在同时 rwx 的区域（违反 W^X，可能被注入代码）
 * - 进一步可以读 /proc/self/smaps，得到每个 VMA 的 Rss、Pss、是否使用 THP 等细节
 *
 * @note 仅 Linux 有 /proc/self/maps；macOS 上会提示改用 `vmmap <pid>`。
 */
class ProcessMapsReader {
public:
    /**
     * @brief 读取 /proc/self/maps 并解析为条目列表。
     * @return 所有映射条目；平台不支持或打开失败时返回空列表
     */
    static std::vector<ProcessMapEntry> read_maps();

    /** @brief 以表格形式打印当前进程的全部映射。 */
    static void print_maps();

    /**
     * @brief 查找包含指定地址的映射条目（相当于内核里的 find_vma）。
     * @param entries read_maps() 的结果
     * @param addr    要查找的地址
     * @return 包含该地址的条目指针（指向 entries 内部，entries 销毁后失效）；找不到返回 nullptr
     */
    static const ProcessMapEntry *find_region(const std::vector<ProcessMapEntry> &entries, void *addr);

private:
    /**
     * @brief 逐行解析 maps 文件。
     * @param f       已打开的 maps 文件
     * @param entries 输出参数，解析结果追加到此列表
     * @note 至少解析出前 6 个字段才算有效行；pathname 可选，最长 255 字符。
     */
    static void parse_maps_file(FILE *f, std::vector<ProcessMapEntry> &entries);
};

/**
 * @brief 阶段 7 的 demo 集合：内核原理观测。
 */
class Stage7Kernel {
public:
    /**
     * @brief Demo 1：用观察者模式追踪一块匿名映射的完整生命周期。
     *
     * 依次执行 mmap → 写数据 → mprotect(R) → mprotect(RW) → msync → mlock → munmap，
     * 每个系统调用都会打印一条事件；中间的直接写内存则不会产生事件。
     *
     * @par 实际应用
     * 帮助建立"哪些操作会进内核、哪些只是普通内存访问"的直觉：
     * mmap / mprotect / munmap 修改的是 VMA 和页表，而写内存只在首次触碰页时
     * 通过缺页进入内核一次，之后都是 CPU 直接访问。
     *
     * @note mlock 可能因 RLIMIT_MEMLOCK 太小而失败，会显示 (failed)，属正常现象。
     */
    static void demo_observer();

    /**
     * @brief Demo 2：创建两块匿名映射，然后在 /proc/self/maps 中找到它们。
     *
     * @par 实际应用
     * 学会读 maps 文件，是排查内存占用、定位崩溃地址、分析库加载情况的基本功。
     * 线上可以用 `cat /proc/<pid>/maps` 或 `pmap -x <pid>` 查看任意进程。
     *
     * @note 两块映射权限不同（rw 和 r），所以即使地址相邻，也会是两个独立的 VMA；
     *       权限相同且相邻的匿名映射，内核可能会把它们合并成一个 VMA。
     * @note macOS 上读不到 maps，会提示使用 `vmmap <pid>`。
     */
    static void demo_process_maps();

    /**
     * @brief Demo 3：讲解 VMA 结构、do_mmap 内核路径和缺页处理（只打印说明）。
     *
     * 核心结论：mmap 是"懒"的 —— 它只在内核里登记一个 VMA 就返回，
     * 物理内存要等到第一次访问触发缺页时才真正分配。
     *
     * @par 实际应用
     * - 解释为什么 mmap 一个 10GB 文件可以瞬间返回，而 RSS 几乎不涨
     * - 解释性能现象：首次遍历映射区较慢（大量缺页），第二次遍历很快
     * - 理解 MAP_POPULATE / madvise(MADV_WILLNEED) / mlock 为什么能减少运行时缺页
     * - 理解 fork 后写时复制：父子进程共享物理页，谁写谁触发 do_wp_page 复制
     *
     * @note 缺页还会受页大小影响：启用透明大页（THP）后，一次缺页可直接映射 2MB，
     *       减少缺页次数和 TLB miss，但可能增加内存浪费和延迟抖动。
     * @note 打印的函数名来自 Linux 内核（6.x），macOS 的 XNU 内核结构完全不同。
     */
    static void demo_vma_concepts();

    /** @brief 依次运行阶段 7 的全部 demo。 */
    static void run_all();
};

} // namespace mmap_learn
