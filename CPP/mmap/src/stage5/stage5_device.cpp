#include "stage5_device.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/ioctl.h>
#endif

namespace mmap_learn {

// ============================================================
// DeviceMapper
// ============================================================

DeviceMapper::~DeviceMapper() {
    // 兜底：正常流程下 unmap_memory() 已释放，这里不会做任何事
    release_resources();
}

void DeviceMapper::release_resources() noexcept {
    if (mapped_addr_ != MAP_FAILED) {
        ::munmap(mapped_addr_, mapped_size_);
        mapped_addr_ = MAP_FAILED;
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

void DeviceMapper::run() {
    if (!open_device()) {
        printf("  [DeviceMapper] open_device() failed, aborting.\n");
        return;
    }
    if (!map_memory()) {
        printf("  [DeviceMapper] map_memory() failed, aborting.\n");
        // 关闭已打开的 fd；若 map_memory() 中途失败但已建立了映射，也一并解除
        release_resources();
        return;
    }
    try {
        process_data();
    } catch (...) {
        // process_data() 抛异常时也要释放映射和 fd，再把异常继续抛出
        unmap_memory();
        throw;
    }
    unmap_memory();
}

// ============================================================
// DevZeroMapper
// ============================================================

bool DevZeroMapper::open_device() {
    printf("  [DevZeroMapper] Opening /dev/zero ...\n");
    fd_ = ::open("/dev/zero", O_RDWR);
    if (fd_ < 0) {
        printf("  [DevZeroMapper] Failed to open /dev/zero: %s\n", strerror(errno));
        return false;
    }
    printf("  [DevZeroMapper] /dev/zero opened, fd = %d\n", fd_);
    return true;
}

size_t DevZeroMapper::page_size() {
    // 运行时查询页大小，而不是写死 4096（Apple Silicon 上是 16KB）
    return static_cast<size_t>(sysconf(_SC_PAGESIZE));
}

bool DevZeroMapper::map_memory() {
    mapped_size_ = page_size();
    printf("  [DevZeroMapper] Mapping %zu bytes from /dev/zero ...\n", mapped_size_);

    // MAP_PRIVATE：得到一页私有、初始为 0 的内存，背后是 /dev/zero。
    // 语义上和 MAP_ANONYMOUS 相同，只是通过设备文件获得。
    mapped_addr_ = ::mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_, 0);
    if (mapped_addr_ == MAP_FAILED) {
        printf("  [DevZeroMapper] mmap failed: %s\n", strerror(errno));
        return false;
    }
    printf("  [DevZeroMapper] Mapped at address %p\n", mapped_addr_);
    return true;
}

void DevZeroMapper::process_data() {
    auto *ptr = static_cast<unsigned char *>(mapped_addr_);

    // 验证 /dev/zero 映射出来的内存确实全是 0
    bool all_zero = true;
    for (size_t i = 0; i < mapped_size_; ++i) {
        if (ptr[i] != 0) {
            all_zero = false;
            break;
        }
    }
    printf("  [DevZeroMapper] Memory is zero-initialized: %s\n", all_zero ? "YES" : "NO");

    // 往映射区写入测试字符串（触发写时复制，生成私有页）
    const char *message = "Hello from /dev/zero mmap!";
    size_t msg_len = strlen(message);
    memcpy(ptr, message, msg_len);
    printf("  [DevZeroMapper] Wrote message: \"%s\"\n", message);

    // 读回确认
    char readback[64] = {};
    memcpy(readback, ptr, msg_len);
    readback[msg_len] = '\0';
    printf("  [DevZeroMapper] Read back:     \"%s\"\n", readback);

    // 在偏移 256 处按 int 写入，说明映射区可以当任意类型的数组使用
    auto *int_ptr = reinterpret_cast<int *>(ptr + 256);
    for (int i = 0; i < 8; ++i) {
        int_ptr[i] = i * 100;
    }
    printf("  [DevZeroMapper] Wrote 8 integers at offset 256: ");
    for (int i = 0; i < 8; ++i) {
        printf("%d ", int_ptr[i]);
    }
    printf("\n");
}

void DevZeroMapper::unmap_memory() {
    if (mapped_addr_ != MAP_FAILED) {
        ::munmap(mapped_addr_, mapped_size_);
        mapped_addr_ = MAP_FAILED;
        printf("  [DevZeroMapper] munmap done.\n");
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
        printf("  [DevZeroMapper] /dev/zero closed.\n");
    }
}

// ============================================================
// FramebufferMapper（仅 Linux）
// ============================================================

#ifdef __linux__
bool FramebufferMapper::open_device() {
    printf("  [FramebufferMapper] Opening /dev/fb0 ...\n");

    // 第 1 步：打开 framebuffer 设备文件。
    // /dev/fb0 是 Linux 上的主 framebuffer。
    // 需要相应权限（通常是 root 或 video 组）。
    fd_ = ::open("/dev/fb0", O_RDWR);
    if (fd_ < 0) {
        printf("  [FramebufferMapper] Failed to open /dev/fb0: %s\n", strerror(errno));
        printf("  [FramebufferMapper] (This is expected if running without a framebuffer device.)\n");
        return false;
    }
    printf("  [FramebufferMapper] /dev/fb0 opened, fd = %d\n", fd_);
    return true;
}

bool FramebufferMapper::map_memory() {
    // 第 2 步：用 ioctl 查询可变屏幕参数。
    // FBIOGET_VSCREENINFO 返回分辨率、每像素位数等。
    if (ioctl(fd_, FBIOGET_VSCREENINFO, &vinfo_) < 0) {
        printf("  [FramebufferMapper] ioctl FBIOGET_VSCREENINFO failed: %s\n", strerror(errno));
        return false;
    }
    printf("  [FramebufferMapper] Resolution: %ux%u, %u bpp\n",
           vinfo_.xres, vinfo_.yres, vinfo_.bits_per_pixel);

    // 第 3 步：用 ioctl 查询固定屏幕参数。
    // FBIOGET_FSCREENINFO 返回显存大小 (smem_len)、每行字节数 (line_length)
    // 以及 framebuffer 类型。smem_len 就是下面 mmap 的长度。
    if (ioctl(fd_, FBIOGET_FSCREENINFO, &finfo_) < 0) {
        printf("  [FramebufferMapper] ioctl FBIOGET_FSCREENINFO failed: %s\n", strerror(errno));
        return false;
    }
    printf("  [FramebufferMapper] Framebuffer size: %u bytes, line length: %u\n",
           static_cast<unsigned>(finfo_.smem_len), static_cast<unsigned>(finfo_.line_length));

    // 第 4 步：把 framebuffer 映射到用户空间。
    // 必须用 MAP_SHARED：写入要直接落到显存上，屏幕才会立刻变化；
    // 若用 MAP_PRIVATE，写入只会改到进程自己的副本，屏幕不会有任何变化。
    mapped_size_ = finfo_.smem_len;
    mapped_addr_ = ::mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (mapped_addr_ == MAP_FAILED) {
        printf("  [FramebufferMapper] mmap failed: %s\n", strerror(errno));
        return false;
    }
    printf("  [FramebufferMapper] Framebuffer mapped at %p (%zu bytes)\n", mapped_addr_, mapped_size_);
    return true;
}

void FramebufferMapper::process_data() {
    // 第 5 步：访问像素数据。
    // 像素 (x, y) 的地址 = base + y * line_length + x * bytes_per_pixel
    // 注意用 line_length 而不是 xres * bpp：每行末尾可能有对齐填充。
    // 往这块内存写数据会直接更新屏幕。

    unsigned bytes_per_pixel = vinfo_.bits_per_pixel / 8;
    printf("  [FramebufferMapper] Bytes per pixel: %u\n", bytes_per_pixel);
    printf("  [FramebufferMapper] To draw a pixel at (x,y):\n");
    printf("    offset = y * line_length + x * bytes_per_pixel\n");
    printf("    ((char*)mapped_addr)[offset + 0] = blue;\n");
    printf("    ((char*)mapped_addr)[offset + 1] = green;\n");
    printf("    ((char*)mapped_addr)[offset + 2] = red;\n");
    printf("    ((char*)mapped_addr)[offset + 3] = alpha;  // if 32bpp\n");

    // 注意：这里故意不画任何东西，以免弄花用户当前的屏幕。
    // 真实应用中会把像素数据写入映射区来渲染画面。
    printf("  [FramebufferMapper] (Skipping actual pixel writes to avoid display corruption.)\n");
}

void FramebufferMapper::unmap_memory() {
    // 第 6 步：清理 —— 解除映射并关闭设备。
    if (mapped_addr_ != MAP_FAILED) {
        ::munmap(mapped_addr_, mapped_size_);
        mapped_addr_ = MAP_FAILED;
        printf("  [FramebufferMapper] munmap done.\n");
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
        printf("  [FramebufferMapper] /dev/fb0 closed.\n");
    }
}
#endif // __linux__

// ============================================================
// Stage5Device
// ============================================================

void Stage5Device::demo_dev_zero() {
    printf("[Stage5] demo_dev_zero: Map /dev/zero for zero-initialized memory\n");
    printf("  /dev/zero provides an infinite source of zero bytes.\n");
    printf("  mmap-ing /dev/zero gives us a zero-initialized private page,\n");
    printf("  similar to MAP_ANONYMOUS but using a real device file.\n\n");

    DevZeroMapper mapper;
    mapper.run();
    printf("\n");
}

void Stage5Device::demo_framebuffer() {
    printf("[Stage5] demo_framebuffer: Framebuffer device mmap\n");

#ifdef __linux__
    printf("  Linux detected. Attempting to open /dev/fb0 ...\n\n");
    FramebufferMapper mapper;
    mapper.run();
#else
    printf("  [SKIPPED] Framebuffer mapping is Linux-specific.\n");
    printf("  On macOS, the display is managed by IOKit/Metal/Core Graphics,\n");
    printf("  not through /dev/fb* device files.\n");
#endif
    printf("\n");
}

void Stage5Device::demo_device_concepts() {
    printf("[Stage5] demo_device_concepts: How mmap is used with device drivers\n\n");

    printf("  === V4L2 (Video4Linux2) ===\n");
    printf("  V4L2 is the Linux kernel API for video capture devices (webcams, etc.).\n");
    printf("  How mmap is used:\n");
    printf("    1. Open the device: open(\"/dev/video0\", O_RDWR)\n");
    printf("    2. Request buffers: ioctl(fd, VIDIOC_REQBUFS, &req)\n");
    printf("       The kernel allocates DMA-capable buffers in kernel space.\n");
    printf("    3. Query buffer info: ioctl(fd, VIDIOC_QUERYBUF, &buf)\n");
    printf("       Returns offset and length for each buffer.\n");
    printf("    4. mmap each buffer: mmap(NULL, buf.length, PROT_READ|PROT_WRITE,\n");
    printf("                              MAP_SHARED, fd, buf.m.offset)\n");
    printf("       Maps the kernel DMA buffer into user space -- zero-copy!\n");
    printf("    5. Queue/dequeue buffers with VIDIOC_QBUF/VIDIOC_DQBUF to\n");
    printf("       stream video frames directly into user-space memory.\n");
    printf("  Benefit: Video frames go from hardware -> kernel DMA -> user space\n");
    printf("           with NO memory copy. Essential for real-time video.\n\n");

    printf("  === DRM/KMS (Direct Rendering Manager / Kernel Mode Setting) ===\n");
    printf("  DRM is the Linux subsystem for GPU access and display management.\n");
    printf("  How mmap is used:\n");
    printf("    1. Open the DRM device: open(\"/dev/dri/card0\", O_RDWR)\n");
    printf("    2. Create a dumb buffer: ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create)\n");
    printf("       Allocates GPU-accessible memory for a framebuffer.\n");
    printf("    3. Prepare for mmap: ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map)\n");
    printf("       Returns a fake offset for use with mmap.\n");
    printf("    4. mmap the buffer: mmap(0, size, PROT_READ|PROT_WRITE,\n");
    printf("                             MAP_SHARED, fd, map.offset)\n");
    printf("       Maps GPU memory into user space for direct pixel access.\n");
    printf("    5. Add framebuffer: ioctl(fd, DRM_IOCTL_MODE_ADDFB, &fb)\n");
    printf("       Registers the buffer as a scanout target for display.\n");
    printf("  Benefit: Direct access to GPU framebuffers without going through\n");
    printf("           a display server. Used by Wayland compositors, kmscon, etc.\n\n");

    printf("  === DMA-BUF (DMA Buffer Sharing) ===\n");
    printf("  DMA-BUF is a kernel framework for sharing buffers between devices.\n");
    printf("  How mmap is used:\n");
    printf("    1. Export a buffer as a DMA-BUF fd from one device (e.g., GPU).\n");
    printf("    2. Import the DMA-BUF fd into another device (e.g., video encoder).\n");
    printf("    3. mmap the DMA-BUF fd: mmap(NULL, size, PROT_READ|PROT_WRITE,\n");
    printf("                                  MAP_SHARED, dmabuf_fd, 0)\n");
    printf("       Maps the shared buffer into user space.\n");
    printf("    4. CPU can now read/write the buffer that is also accessible by\n");
    printf("       multiple hardware devices (GPU, display, video codec, camera).\n");
    printf("  Benefit: Zero-copy buffer sharing across different hardware devices.\n");
    printf("           A single buffer can flow: Camera -> GPU -> Display\n");
    printf("           without any CPU-side memcpy.\n\n");

    printf("  === Summary ===\n");
    printf("  In all these cases, mmap serves the same fundamental purpose:\n");
    printf("    - Map device/kernel memory into user-space address space\n");
    printf("    - Achieve zero-copy data transfer between hardware and software\n");
    printf("    - The driver implements the file_operations.mmap callback\n");
    printf("      to set up the page table entries for the device memory region\n\n");
}

void Stage5Device::run_all() {
    printf("========================================\n");
    printf("  Stage 5: Device/Driver-Level mmap\n");
    printf("========================================\n\n");

    demo_dev_zero();
    demo_framebuffer();
    demo_device_concepts();

    printf("========================================\n");
    printf("  Stage 5 complete\n");
    printf("========================================\n");
}

} // namespace mmap_learn
