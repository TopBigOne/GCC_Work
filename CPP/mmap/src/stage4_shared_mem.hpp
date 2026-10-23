#pragma once

#include "mmap_wrapper.hpp"

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sys/wait.h>

namespace mmap_learn {

// ============================================================
// RAII: POSIX shared memory (shm_open + mmap + shm_unlink)
// ============================================================
class SharedMemory {
    void *addr_ = MAP_FAILED;
    size_t size_ = 0;
    std::string name_;
    bool owner_ = false; // true = creator, responsible for shm_unlink

public:
    /// @param name   shared memory name (must start with '/')
    /// @param size   mapping size in bytes
    /// @param create true = create new shm, false = open existing
    SharedMemory(const char *name, size_t size, bool create)
        : size_(size), name_(name), owner_(create) {
        int fd = -1;
        if (create) {
            fd = ::shm_open(name, O_CREAT | O_RDWR, 0666);
            if (fd < 0) {
                throw std::runtime_error(std::string("shm_open create failed: ") + strerror(errno));
            }
            if (::ftruncate(fd, static_cast<off_t>(size)) < 0) {
                ::close(fd);
                ::shm_unlink(name);
                throw std::runtime_error(std::string("ftruncate failed: ") + strerror(errno));
            }
        } else {
            fd = ::shm_open(name, O_RDWR, 0666);
            if (fd < 0) {
                throw std::runtime_error(std::string("shm_open open failed: ") + strerror(errno));
            }
        }

        addr_ = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        ::close(fd); // fd no longer needed after mmap
        if (addr_ == MAP_FAILED) {
            if (create) ::shm_unlink(name);
            throw std::runtime_error(std::string("mmap failed: ") + strerror(errno));
        }
    }

    ~SharedMemory() {
        if (addr_ != MAP_FAILED) {
            ::munmap(addr_, size_);
        }
        if (owner_) {
            ::shm_unlink(name_.c_str());
        }
    }

    // No copy
    SharedMemory(const SharedMemory &) = delete;
    SharedMemory &operator=(const SharedMemory &) = delete;

    // Move OK
    SharedMemory(SharedMemory &&other) noexcept
        : addr_(other.addr_), size_(other.size_), name_(std::move(other.name_)), owner_(other.owner_) {
        other.addr_ = MAP_FAILED;
        other.size_ = 0;
        other.owner_ = false;
    }

    SharedMemory &operator=(SharedMemory &&other) noexcept {
        if (this != &other) {
            if (addr_ != MAP_FAILED) ::munmap(addr_, size_);
            if (owner_) ::shm_unlink(name_.c_str());

            addr_ = other.addr_;
            size_ = other.size_;
            name_ = std::move(other.name_);
            owner_ = other.owner_;

            other.addr_ = MAP_FAILED;
            other.size_ = 0;
            other.owner_ = false;
        }
        return *this;
    }

    void *data() { return addr_; }
    const void *data() const { return addr_; }
    size_t size() const { return size_; }

    template <typename T>
    T *as() {
        return static_cast<T *>(addr_);
    }

    template <typename T>
    const T *as() const {
        return static_cast<const T *>(addr_);
    }
};

// ============================================================
// Lock-free SPSC ring buffer for shared memory
// ============================================================
template <typename T, uint32_t N>
struct RingBuffer {
    static_assert(std::is_trivially_copyable<T>::value, "RingBuffer<T> requires trivially copyable T");
    static_assert(N > 0, "RingBuffer capacity must be > 0");

    std::atomic<uint32_t> head_{0}; // written by producer
    std::atomic<uint32_t> tail_{0}; // written by consumer
    T data_[N];

    /// Push an item (producer side). Returns false if full.
    bool push(const T &item) {
        uint32_t h = head_.load(std::memory_order_relaxed);
        uint32_t next = (h + 1) % N;
        if (next == tail_.load(std::memory_order_acquire)) {
            return false; // full
        }
        data_[h] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    /// Pop an item (consumer side). Returns false if empty.
    bool pop(T &item) {
        uint32_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire)) {
            return false; // empty
        }
        item = data_[t];
        tail_.store((t + 1) % N, std::memory_order_release);
        return true;
    }

    bool empty() const { return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire); }

    bool full() const {
        uint32_t next = (head_.load(std::memory_order_acquire) + 1) % N;
        return next == tail_.load(std::memory_order_acquire);
    }

    uint32_t count() const {
        uint32_t h = head_.load(std::memory_order_acquire);
        uint32_t t = tail_.load(std::memory_order_acquire);
        return (h >= t) ? (h - t) : (N - t + h);
    }
};

// ============================================================
// Stage4SharedMem: demo methods
// ============================================================
class Stage4SharedMem {
public:
    // ---------------------------------------------------------
    // Demo 1: MAP_SHARED anonymous mapping across fork
    // ---------------------------------------------------------
    static void demo_fork_shared() {
        printf("\n===== Stage4: demo_fork_shared =====\n");

        const size_t kSize = 4096;
        MmapRegion region(nullptr, kSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

        char *buf = region.as<char>();
        std::memset(buf, 0, kSize);

        pid_t pid = ::fork();
        if (pid < 0) {
            perror("fork");
            return;
        }

        if (pid == 0) {
            // Child: write a message into shared memory
            const char *msg = "Hello from child process!";
            std::memcpy(buf, msg, std::strlen(msg) + 1);
            printf("  [child ] wrote: \"%s\"\n", buf);
            ::_exit(0);
        } else {
            // Parent: wait for child, then read
            int status = 0;
            ::waitpid(pid, &status, 0);
            printf("  [parent] read : \"%s\"\n", buf);
        }
    }

    // ---------------------------------------------------------
    // Demo 2: POSIX shm_open shared memory across fork
    // ---------------------------------------------------------
    static void demo_shm_open() {
        printf("\n===== Stage4: demo_shm_open =====\n");

        const char *kName = "/mmap_learn_stage4";
        const size_t kSize = 4096;

        // Parent creates the shared memory
        SharedMemory shm(kName, kSize, true);
        std::memset(shm.data(), 0, kSize);

        pid_t pid = ::fork();
        if (pid < 0) {
            perror("fork");
            return;
        }

        if (pid == 0) {
            // Child: open the same shared memory by name
            SharedMemory child_shm(kName, kSize, false);
            const char *msg = "Data from child via shm_open!";
            std::memcpy(child_shm.data(), msg, std::strlen(msg) + 1);
            printf("  [child ] wrote: \"%s\"\n", static_cast<char *>(child_shm.data()));
            ::_exit(0);
        } else {
            int status = 0;
            ::waitpid(pid, &status, 0);
            printf("  [parent] read : \"%s\"\n", static_cast<char *>(shm.data()));
        }
    }

    // ---------------------------------------------------------
    // Demo 3: Ring buffer in shared memory
    // ---------------------------------------------------------
    static void demo_ring_buffer() {
        printf("\n===== Stage4: demo_ring_buffer =====\n");

        using Ring = RingBuffer<int, 16>;

        const char *kName = "/mmap_learn_ring";
        const size_t kSize = sizeof(Ring);

        SharedMemory shm(kName, kSize, true);

        // Placement-new the ring buffer into shared memory
        Ring *ring = new (shm.data()) Ring();

        pid_t pid = ::fork();
        if (pid < 0) {
            perror("fork");
            return;
        }

        if (pid == 0) {
            // Child: push 10 numbers
            for (int i = 0; i < 10; ++i) {
                while (!ring->push(i * 10)) {
                    ::usleep(100); // back off if full
                }
                printf("  [child ] pushed: %d\n", i * 10);
            }
            ::_exit(0);
        } else {
            int status = 0;
            ::waitpid(pid, &status, 0);

            // Parent: pop all items after child exits
            int val = 0;
            int popped = 0;
            while (ring->pop(val)) {
                printf("  [parent] popped: %d\n", val);
                ++popped;
            }
            printf("  [parent] total popped: %d items\n", popped);
        }
    }

    // ---------------------------------------------------------
    // Run all demos
    // ---------------------------------------------------------
    static void run_all() {
        printf("========================================\n");
        printf("  Stage 4: IPC Shared Memory\n");
        printf("========================================\n");

        demo_fork_shared();
        demo_shm_open();
        demo_ring_buffer();

        printf("\n===== Stage4: all demos complete =====\n");
    }
};

} // namespace mmap_learn
