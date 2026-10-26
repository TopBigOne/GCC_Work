/**
 * @file stage5_device.h
 * @brief 阶段 5：设备与驱动层 mmap —— 把"设备文件"映射进用户空间。
 *
 * 前几个阶段映射的是普通文件或匿名内存；本阶段映射的是设备文件
 * （/dev/zero、/dev/fb0、/dev/video0、/dev/dri/card0、dma-buf fd 等）。
 * 对设备 fd 调用 mmap 时，内核会转给驱动的 file_operations.mmap 回调，
 * 由驱动决定这段虚拟地址最终指向哪里：可能是显存、摄像头 DMA 缓冲、
 * 硬件寄存器，或者干脆是零页。
 *
 * 对应知识点：
 * - 设备 mmap 的调用路径：mmap(2) → do_mmap → 驱动 fops->mmap → remap_pfn_range / vm_ops->fault
 * - ioctl + mmap 配合：先用 ioctl 查询/申请缓冲区（拿到大小、offset），再 mmap
 * - MAP_SHARED 是设备映射的常态：写入要直达硬件，不能写时复制
 * - 缓存一致性：设备内存常映射为 uncached / write-combine，DMA 前后需要同步
 *
 * @par 实际应用
 * - 摄像头采集（V4L2）：内核 DMA 缓冲直接映射给应用，视频帧零拷贝进入用户态
 * - 嵌入式显示（fbdev / DRM dumb buffer）：直接往显存写像素，无需窗口系统
 * - GPU / 编解码器间共享缓冲（DMA-BUF）：Camera → GPU → Display 全程不经 CPU 拷贝
 * - 驱动开发：实现 fops->mmap，让用户态以访问内存的方式访问硬件寄存器（UIO、FPGA）
 *
 * @note /dev/zero demo 在 Linux 上可以完整运行；macOS 的 /dev/zero 不支持 mmap
 *       （返回 ENODEV "Operation not supported by device"），demo 会打印失败后退出。
 *       framebuffer 相关代码用 `#ifdef __linux__` 包裹，macOS 上会跳过
 *       （macOS 的显示由 IOKit / Metal 管理，没有 /dev/fb*）。
 */
#pragma once

#include "../mmap_wrapper.hpp"

#include <cstddef>
#include <sys/mman.h>

#ifdef __linux__
#include <linux/fb.h> // FramebufferMapper 的成员类型 fb_var_screeninfo / fb_fix_screeninfo
#endif

namespace mmap_learn {

/**
 * @brief 设备映射的抽象基类（模板方法模式）。
 *
 * 所有设备 mmap 都遵循同一套流程：打开设备 → (ioctl 查询) → mmap → 读写数据 → munmap + close。
 * 基类在 run() 中固定这套流程，子类只需实现各步骤的具体做法。
 *
 * @par 实际应用
 * 真实的设备访问代码（摄像头、显示、FPGA 寄存器）结构几乎一样，差别只在
 * 设备路径、ioctl 命令和数据处理方式。把流程抽到基类，可以避免每种设备
 * 重复写打开/映射/清理逻辑，也不容易漏掉 munmap / close。
 *
 * @note 这里直接使用原始 fd 和 ::mmap，而不是 FileDescriptor / MmapRegion，
 *       目的是让每一步系统调用在子类中清晰可见。为保证异常安全：run() 在
 *       process_data() 抛异常时仍会调用 unmap_memory()；析构函数再通过非虚的
 *       release_resources() 兜底释放尚未释放的映射和 fd。
 */
class DeviceMapper {
protected:
    int fd_ = -1;                    ///< 设备文件描述符，-1 表示未打开
    void *mapped_addr_ = MAP_FAILED; ///< 映射起始地址，MAP_FAILED 表示未映射
    size_t mapped_size_ = 0;         ///< 映射长度（字节）

    /**
     * @brief 打开设备文件，结果保存在 fd_。
     * @return 成功返回 true
     */
    virtual bool open_device() = 0;

    /**
     * @brief 查询设备参数并 mmap，结果保存在 mapped_addr_ / mapped_size_。
     * @return 成功返回 true；失败时 run() 会负责关闭 fd_
     */
    virtual bool map_memory() = 0;

    /** @brief 通过映射地址读写设备数据。 */
    virtual void process_data() = 0;

    /**
     * @brief munmap 并关闭设备。
     * @note 实现中释放后需把 mapped_addr_ 置为 MAP_FAILED、fd_ 置为 -1，
     *       这样析构时的 release_resources() 不会重复释放。
     */
    virtual void unmap_memory() = 0;

    /**
     * @brief 直接 munmap 映射并 close fd（非虚函数，不打印日志），可重复调用。
     *
     * 供析构函数和 run() 的失败路径使用。之所以不调用虚函数 unmap_memory()：
     * 在基类析构函数里，子类部分已经销毁，虚调用不会分派到子类实现。
     */
    void release_resources() noexcept;

public:
    /** @brief 默认构造：尚未打开设备，也没有映射。 */
    DeviceMapper() = default;

    /** @brief 禁止拷贝：fd 和映射只能有一个所有者，拷贝会导致重复释放。 */
    DeviceMapper(const DeviceMapper &) = delete;

    /** @brief 禁止拷贝赋值，理由同拷贝构造。 */
    DeviceMapper &operator=(const DeviceMapper &) = delete;

    /**
     * @brief 虚析构函数：通过基类指针删除子类对象时行为正确，并兜底释放资源。
     *
     * 正常流程中 run() 已经调用 unmap_memory()，这里什么都不做；
     * 若中途异常或提前返回，则由 release_resources() 释放剩余的映射和 fd。
     */
    virtual ~DeviceMapper();

    /**
     * @brief 模板方法：按固定顺序执行 open → map → process → unmap。
     *
     * 任一步失败即中止；map_memory() 失败时会释放已打开的 fd（以及可能已建立的映射）。
     * @throw 透传 process_data() 抛出的异常；抛出前会先调用 unmap_memory() 释放资源。
     */
    void run();
};

/**
 * @brief 映射 /dev/zero，得到一页全零、可读写的私有内存。
 *
 * /dev/zero 是一个"读出来永远是 0"的字符设备。用 MAP_PRIVATE 映射它，
 * 效果和 MAP_ANONYMOUS 基本一样：拿到一块初始为 0 的内存，写入时
 * 写时复制出私有页，不会影响任何文件。
 *
 * @par 实际应用
 * - 在 MAP_ANONYMOUS 出现之前，老代码（以及一些老 Unix）就是用
 *   mmap("/dev/zero") 来申请大块清零内存的，读老代码时会遇到这种写法
 * - 在 Linux 上，父子进程对 /dev/zero 做 MAP_SHARED 映射，也能实现共享内存
 * - 作为最简单的设备映射例子，演示"对设备 fd 调用 mmap"的完整流程
 *
 * @note Linux 支持；macOS 的 /dev/zero 驱动没有实现 mmap，map_memory() 会以 ENODEV 失败。
 */
class DevZeroMapper : public DeviceMapper {
    /**
     * @brief 获取系统页大小（sysconf(_SC_PAGESIZE)），作为映射长度。
     * @return 页大小（字节）：x86-64 Linux 通常 4096，Apple Silicon macOS 为 16384
     */
    static size_t page_size();

protected:
    /**
     * @brief 以读写方式打开 /dev/zero。
     * @return 成功返回 true
     */
    bool open_device() override;

    /**
     * @brief 以 PROT_READ | PROT_WRITE + MAP_PRIVATE 映射一页 /dev/zero（长度取运行时页大小）。
     * @return 成功返回 true
     */
    bool map_memory() override;

    /**
     * @brief 验证映射内容全为 0，再写入字符串和整数并读回，证明它就是普通可写内存。
     */
    void process_data() override;

    /** @brief munmap 映射并关闭 /dev/zero。 */
    void unmap_memory() override;
};

#ifdef __linux__
/**
 * @brief 映射 Linux framebuffer（/dev/fb0），直接访问显存。
 *
 * 流程：open("/dev/fb0") → ioctl 查询分辨率、每像素位数和显存大小 →
 * 以 MAP_SHARED 映射整块显存 → 往映射区写像素即可直接改变屏幕内容。
 *
 * @par 实际应用
 * - 嵌入式设备（工控屏、POS 机、车机启动画面）没有 X11/Wayland，
 *   应用直接写 framebuffer 来画界面
 * - 开机 logo、恢复模式界面、终端模拟器（fbterm）
 * - 理解"显存映射到用户空间"之后，再学 DRM/KMS 的 dumb buffer 会很自然
 *
 * @note 驱动侧通常用 remap_pfn_range 把显存的物理页帧号（PFN）填进进程页表，
 *       并把这段映射设置为 write-combine，保证写入能尽快到达显示硬件。
 * @note 仅 Linux 可用；需要 root 或 video 组权限，现代桌面发行版也可能没有 /dev/fb0。
 */
class FramebufferMapper : public DeviceMapper {
    struct fb_var_screeninfo vinfo_ {}; ///< 可变屏幕参数：分辨率、bits_per_pixel 等
    struct fb_fix_screeninfo finfo_ {}; ///< 固定屏幕参数：显存大小 smem_len、每行字节数 line_length

protected:
    /**
     * @brief 以读写方式打开 /dev/fb0。
     * @return 成功返回 true；没有 framebuffer 设备或权限不足时返回 false
     */
    bool open_device() override;

    /**
     * @brief 用 ioctl 查询屏幕参数，再以 MAP_SHARED 映射整块显存。
     * @return 成功返回 true
     */
    bool map_memory() override;

    /**
     * @brief 打印像素寻址公式；为避免破坏当前屏幕，实际并不写像素。
     */
    void process_data() override;

    /** @brief munmap 显存映射并关闭 /dev/fb0。 */
    void unmap_memory() override;
};
#endif // __linux__

/**
 * @brief 阶段 5 的 demo 集合：设备与驱动层 mmap。
 */
class Stage5Device {
public:
    /**
     * @brief Demo 1：映射 /dev/zero，体验最简单的设备 mmap。
     *
     * @par 实际应用
     * 理解"对设备 fd 调 mmap"这件事本身；同时认识 /dev/zero 这种
     * 老式的申请清零内存的写法。
     * @note Linux 上完整运行；macOS 上 mmap(/dev/zero) 返回 ENODEV，打印失败信息后退出。
     */
    static void demo_dev_zero();

    /**
     * @brief Demo 2：映射 framebuffer，直接访问显存。
     *
     * @par 实际应用
     * 嵌入式 Linux 无窗口系统时的绘图方式；开机画面、工控界面等。
     * @note 仅 Linux；macOS 上打印说明后跳过。Linux 上没有 /dev/fb0 或权限不足时也会失败退出。
     */
    static void demo_framebuffer();

    /**
     * @brief Demo 3：讲解 V4L2、DRM/KMS、DMA-BUF 三种典型设备 mmap 的流程（只打印，不实际操作设备）。
     *
     * 三者模式相同：先用 ioctl 让驱动分配缓冲并返回 offset，
     * 再用 mmap(..., MAP_SHARED, fd, offset) 把这块内核/设备内存映射到用户空间。
     * 这里的 offset 并不是真实的文件偏移，而是驱动约定的"缓冲区编号"（fake offset）。
     *
     * @par 实际应用
     * - V4L2：USB 摄像头、工业相机采集，帧数据零拷贝到用户态再做图像处理
     * - DRM/KMS：Wayland 合成器、无桌面的全屏显示程序直接管理显示输出
     * - DMA-BUF：Android / Linux 多媒体管线中，相机、GPU、编码器、显示之间共享同一块缓冲
     *
     * @note 驱动侧实现 file_operations.mmap，常用 remap_pfn_range 一次性填好页表，
     *       或者提供 vm_operations_struct.fault，在缺页时再按需插入页。
     * @note CPU 与 DMA 设备同时访问同一缓冲时要注意缓存一致性：DMA-BUF 需要用
     *       DMA_BUF_IOCTL_SYNC 包住 CPU 访问。
     */
    static void demo_device_concepts();

    /** @brief 依次运行阶段 5 的全部 demo。 */
    static void run_all();
};

} // namespace mmap_learn
