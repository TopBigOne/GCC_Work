#pragma once

#include "mmap_wrapper.hpp"

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/fb.h>
#include <sys/ioctl.h>
#endif

namespace mmap_learn {

// ============================================================
// Abstract base class: Template Method pattern for device mmap
// ============================================================
class DeviceMapper {
protected:
    int fd_ = -1;
    void *mapped_addr_ = MAP_FAILED;
    size_t mapped_size_ = 0;

    virtual bool open_device() = 0;
    virtual bool map_memory() = 0;
    virtual void process_data() = 0;
    virtual void unmap_memory() = 0;

public:
    virtual ~DeviceMapper() = default;

    // Template Method: defines the skeleton algorithm
    void run() {
        if (!open_device()) {
            printf("  [DeviceMapper] open_device() failed, aborting.\n");
            return;
        }
        if (!map_memory()) {
            printf("  [DeviceMapper] map_memory() failed, aborting.\n");
            if (fd_ >= 0) {
                ::close(fd_);
                fd_ = -1;
            }
            return;
        }
        process_data();
        unmap_memory();
    }
};

// ============================================================
// DevZeroMapper: Map /dev/zero for zero-initialized memory
// Works on both macOS and Linux.
// ============================================================
class DevZeroMapper : public DeviceMapper {
    static constexpr size_t kPageSize = 4096;

protected:
    bool open_device() override {
        printf("  [DevZeroMapper] Opening /dev/zero ...\n");
        fd_ = ::open("/dev/zero", O_RDWR);
        if (fd_ < 0) {
            printf("  [DevZeroMapper] Failed to open /dev/zero: %s\n", strerror(errno));
            return false;
        }
        printf("  [DevZeroMapper] /dev/zero opened, fd = %d\n", fd_);
        return true;
    }

    bool map_memory() override {
        mapped_size_ = kPageSize;
        printf("  [DevZeroMapper] Mapping %zu bytes from /dev/zero ...\n", mapped_size_);

        // MAP_PRIVATE: we get a private, zero-initialized page backed by /dev/zero.
        // This is semantically similar to MAP_ANONYMOUS but uses the device file.
        mapped_addr_ = ::mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd_, 0);
        if (mapped_addr_ == MAP_FAILED) {
            printf("  [DevZeroMapper] mmap failed: %s\n", strerror(errno));
            return false;
        }
        printf("  [DevZeroMapper] Mapped at address %p\n", mapped_addr_);
        return true;
    }

    void process_data() override {
        auto *ptr = static_cast<unsigned char *>(mapped_addr_);

        // Verify that memory from /dev/zero is indeed zero-initialized
        bool all_zero = true;
        for (size_t i = 0; i < mapped_size_; ++i) {
            if (ptr[i] != 0) {
                all_zero = false;
                break;
            }
        }
        printf("  [DevZeroMapper] Memory is zero-initialized: %s\n", all_zero ? "YES" : "NO");

        // Write a test pattern into the mapped region
        const char *message = "Hello from /dev/zero mmap!";
        size_t msg_len = strlen(message);
        memcpy(ptr, message, msg_len);
        printf("  [DevZeroMapper] Wrote message: \"%s\"\n", message);

        // Read it back to confirm
        char readback[64] = {};
        memcpy(readback, ptr, msg_len);
        readback[msg_len] = '\0';
        printf("  [DevZeroMapper] Read back:     \"%s\"\n", readback);

        // Write an integer pattern at a different offset
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

    void unmap_memory() override {
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
};

// ============================================================
// FramebufferMapper: Conceptual framebuffer mmap (Linux only)
// ============================================================
#ifdef __linux__
class FramebufferMapper : public DeviceMapper {
    struct fb_var_screeninfo vinfo_ {};
    struct fb_fix_screeninfo finfo_ {};

protected:
    bool open_device() override {
        printf("  [FramebufferMapper] Opening /dev/fb0 ...\n");

        // Step 1: Open the framebuffer device file.
        // /dev/fb0 is the primary framebuffer on Linux systems.
        // Requires appropriate permissions (typically root or video group).
        fd_ = ::open("/dev/fb0", O_RDWR);
        if (fd_ < 0) {
            printf("  [FramebufferMapper] Failed to open /dev/fb0: %s\n", strerror(errno));
            printf("  [FramebufferMapper] (This is expected if running without a framebuffer device.)\n");
            return false;
        }
        printf("  [FramebufferMapper] /dev/fb0 opened, fd = %d\n", fd_);
        return true;
    }

    bool map_memory() override {
        // Step 2: Query variable screen info via ioctl.
        // FBIOGET_VSCREENINFO returns resolution, bits per pixel, etc.
        if (ioctl(fd_, FBIOGET_VSCREENINFO, &vinfo_) < 0) {
            printf("  [FramebufferMapper] ioctl FBIOGET_VSCREENINFO failed: %s\n", strerror(errno));
            return false;
        }
        printf("  [FramebufferMapper] Resolution: %ux%u, %u bpp\n",
               vinfo_.xres, vinfo_.yres, vinfo_.bits_per_pixel);

        // Step 3: Query fixed screen info via ioctl.
        // FBIOGET_FSCREENINFO returns the size of the framebuffer memory (smem_len),
        // line length (line_length), and the type of framebuffer.
        if (ioctl(fd_, FBIOGET_FSCREENINFO, &finfo_) < 0) {
            printf("  [FramebufferMapper] ioctl FBIOGET_FSCREENINFO failed: %s\n", strerror(errno));
            return false;
        }
        printf("  [FramebufferMapper] Framebuffer size: %u bytes, line length: %u\n",
               static_cast<unsigned>(finfo_.smem_len), static_cast<unsigned>(finfo_.line_length));

        // Step 4: mmap the framebuffer into user space.
        // MAP_SHARED is required so that writes to the mapped region
        // are immediately visible on the display (backed by GPU/display memory).
        mapped_size_ = finfo_.smem_len;
        mapped_addr_ = ::mmap(nullptr, mapped_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mapped_addr_ == MAP_FAILED) {
            printf("  [FramebufferMapper] mmap failed: %s\n", strerror(errno));
            return false;
        }
        printf("  [FramebufferMapper] Framebuffer mapped at %p (%zu bytes)\n", mapped_addr_, mapped_size_);
        return true;
    }

    void process_data() override {
        // Step 5: Access framebuffer pixel data.
        // Each pixel is at: base + (y * line_length) + (x * bytes_per_pixel)
        // Writing to this memory directly updates the display.

        unsigned bytes_per_pixel = vinfo_.bits_per_pixel / 8;
        printf("  [FramebufferMapper] Bytes per pixel: %u\n", bytes_per_pixel);
        printf("  [FramebufferMapper] To draw a pixel at (x,y):\n");
        printf("    offset = y * line_length + x * bytes_per_pixel\n");
        printf("    ((char*)mapped_addr)[offset + 0] = blue;\n");
        printf("    ((char*)mapped_addr)[offset + 1] = green;\n");
        printf("    ((char*)mapped_addr)[offset + 2] = red;\n");
        printf("    ((char*)mapped_addr)[offset + 3] = alpha;  // if 32bpp\n");

        // NOTE: We intentionally do NOT draw anything here to avoid
        // corrupting the user's display. In a real application, you would
        // write pixel data to the mapped region to render graphics.
        printf("  [FramebufferMapper] (Skipping actual pixel writes to avoid display corruption.)\n");
    }

    void unmap_memory() override {
        // Step 6: Clean up - unmap the framebuffer and close the device.
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
};
#endif // __linux__

// ============================================================
// Stage5Device: Demonstrate device/driver-level mmap concepts
// ============================================================
class Stage5Device {
public:
    // ============================================================
    // Demo 1: /dev/zero mapping (cross-platform)
    // ============================================================
    static void demo_dev_zero() {
        printf("[Stage5] demo_dev_zero: Map /dev/zero for zero-initialized memory\n");
        printf("  /dev/zero provides an infinite source of zero bytes.\n");
        printf("  mmap-ing /dev/zero gives us a zero-initialized private page,\n");
        printf("  similar to MAP_ANONYMOUS but using a real device file.\n\n");

        DevZeroMapper mapper;
        mapper.run();
        printf("\n");
    }

    // ============================================================
    // Demo 2: Framebuffer mapping (Linux only)
    // ============================================================
    static void demo_framebuffer() {
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

    // ============================================================
    // Demo 3: Device mmap concepts (educational)
    // ============================================================
    static void demo_device_concepts() {
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

    // ============================================================
    // Run all Stage 5 demos
    // ============================================================
    static void run_all() {
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
};

} // namespace mmap_learn
