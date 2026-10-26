#include "stage4_shared_mem.h"

#include "../log.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <new>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>

namespace mmap_learn {

namespace {
constexpr const char* kTag = "Stage4";
} // namespace

    // ============================================================
    // SharedMemory
    // ============================================================

    SharedMemory::SharedMemory(const char *name, size_t size, bool create) : size_(size), name_(name), owner_(create) {
        int fd = -1;
        if (create) {
            fd = ::shm_open(name, O_CREAT | O_RDWR, 0666);
            if (fd < 0) {
                throw std::runtime_error(std::string("shm_open create failed: ") + strerror(errno));
            }
            // 新建的共享内存对象大小为 0，必须先设定大小再映射，否则访问会触发 SIGBUS
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

        // 必须用 MAP_SHARED，MAP_PRIVATE 会写时复制，其他进程看不到修改
        mapped_addr_ = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        ::close(fd); // mmap 之后 fd 不再需要，映射依然有效
        if (mapped_addr_ == MAP_FAILED) {
            if (create)
                ::shm_unlink(name);
            throw std::runtime_error(std::string("mmap failed: ") + strerror(errno));
        }
    }

    SharedMemory::~SharedMemory() {
        if (mapped_addr_ != MAP_FAILED) {
            ::munmap(mapped_addr_, size_);
        }
        if (owner_) {
            ::shm_unlink(name_.c_str());
        }
    }

    SharedMemory::SharedMemory(SharedMemory &&other) noexcept :
        mapped_addr_(other.mapped_addr_), size_(other.size_), name_(std::move(other.name_)), owner_(other.owner_) {
        other.mapped_addr_ = MAP_FAILED;
        other.size_ = 0;
        other.owner_ = false;
    }

    SharedMemory &SharedMemory::operator=(SharedMemory &&other) noexcept {
        if (this != &other) {
            if (mapped_addr_ != MAP_FAILED)
                ::munmap(mapped_addr_, size_);
            if (owner_)
                ::shm_unlink(name_.c_str());

            mapped_addr_ = other.mapped_addr_;
            size_ = other.size_;
            name_ = std::move(other.name_);
            owner_ = other.owner_;

            other.mapped_addr_ = MAP_FAILED;
            other.size_ = 0;
            other.owner_ = false;
        }
        return *this;
    }

    void *SharedMemory::data() { return mapped_addr_; }

    const void *SharedMemory::data() const { return mapped_addr_; }

    size_t SharedMemory::size() const { return size_; }

    // ============================================================
    // Stage4SharedMem
    // ============================================================

    /**
     * MAP_SHARED | MAP_ANON 匿名共享映射，fork 后父子进程共享。
     *
     * 子进程写入字符串后退出，父进程等待后读到同样的内容。
     */
    void Stage4SharedMem::demo_fork_shared() {
        LOGI(kTag, "===== Stage4-1: demo_fork_shared =====");

        const size_t kSize = 4096;
        MmapRegion region(nullptr, kSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

        char *buf = region.as<char>();
        std::memset(buf, 0, kSize);

        // fork 在父进程里返回子进程的 PID，在子进程里返回 0，失败返回 -1
        pid_t child_pid = fork();
        if (child_pid < 0) {
            LOGE(kTag, "fork in error: %s", strerror(errno));
            return;
        }

        if (child_pid == 0) {
            // 子进程：向共享内存写入字符串
            const char *msg = "Hello from child process!";
            std::memcpy(buf, msg, std::strlen(msg) + 1);
            LOGD(kTag, "[child ] wrote: \"%s\"", buf);
            // 用 _exit 而不是 exit/return：不执行析构和 atexit，避免子进程重复释放父进程的资源
            ::_exit(0);
        } else {
            // 父进程：等待子进程结束后再读取
            int status = 0;
            ::waitpid(child_pid, &status, 0);
            LOGD(kTag, "[parent] read : \"%s\"", buf);
        }
    }

    /**
     * 通过 shm_open 按名字打开同一块共享内存。
     */
    void Stage4SharedMem::demo_shm_open() {
        LOGI(kTag, "===== Stage4-2: demo_shm_open =====");

        const char *kName = "/mmap_learn_stage4";
        const size_t kSize = 4096;

        // 父进程创建共享内存（owner，析构时 shm_unlink）
        SharedMemory shm(kName, kSize, true);
        std::memset(shm.data(), 0, kSize);

        pid_t child_pid = fork();
        if (child_pid < 0) {
            LOGE(kTag, "fork in error: %s", strerror(errno));
            return;
        }

        if (child_pid == 0) {
            // 子进程：按名字打开同一块共享内存
            SharedMemory child_shm(kName, kSize, false);
            const char *rawMsg = "Data from child via shm_open!";
            std::memcpy(child_shm.data(), rawMsg, std::strlen(rawMsg) + 1);
            LOGD(kTag, "[child ] wrote: \"%s\"", static_cast<char *>(child_shm.data()));
            // 子进程也继承了父进程的 shm（owner_ == true），用 _exit 跳过析构，避免子进程提前 shm_unlink
            ::_exit(0);
        } else {
            int status = 0;
            ::waitpid(child_pid, &status, 0);
            LOGD(kTag, "[parent] read : \"%s\"", static_cast<char *>(shm.data()));
        }
    }

    void Stage4SharedMem::demo_ring_buffer() {
        LOGI(kTag, "===== Stage4-3: demo_ring_buffer =====");

        // 类型别名（等价于 typedef RingBuffer<int, 16> Ring;）：之后写 Ring 就等于写 RingBuffer<int, 16>。
        //   int : 队列里存放的元素类型
        //   16  : 槽位数 N；空出一格用来区分"满"和"空"，所以最多同时存 15 个
        // 布局为 head_(4B) + tail_(4B) + data_[16](64B) = 72 字节，下面的 sizeof(Ring) 就是这个值
        using Ring = RingBuffer<int, 16>;

        const char *kName = "/mmap_learn_ring";
        const size_t kSize = sizeof(Ring);

        SharedMemory shm(kName, kSize, true);

        // 在共享内存上 placement new 构造环形队列（初始化 head_/tail_ 为 0）
        // 有括号：不找新内存，直接用 shm.data() 这个地址
        Ring *ring = new (shm.data()) Ring();
        //  Ring *ring2 = new  Ring();//没有括号：new 自己去堆上找一块新内存


        // 生产者要写入的条目数；可以大于队列容量（N-1），因为父进程会同时消费
        const int kCount = 10;
        // fork 前清空 stdio 缓冲，避免缓冲区内容被复制到子进程后重复输出
        fflush(stdout);
        pid_t child_pid = fork();
        // Note: 因为开辟了子进程，下面代码会走2遍
        LOGD(kTag, "---> sw sw sw....");
        fflush(stdout);
        if (child_pid < 0) {
            LOGE(kTag, "fork: %s", strerror(errno));
            return;
        }

        // 进程里的 child_pid 是子进程的 PID，是一个正数。只有在子进程里，它的值才是 0。
        if (child_pid == 0) {
            // 在子进程里，getpid() 拿到的是自己的 PID
            pid_t childPid = getpid();
            pid_t parentPid = getppid();
            LOGD(kTag, "childPid : %d", childPid);
            LOGD(kTag, "child_pid   : %d", child_pid);
            LOGD(kTag, "parentPid2 : %d", parentPid);
            // 子进程（生产者）：写入 kCount 个数
            for (int i = 0; i < kCount; ++i) {
                while (!ring->push(i * 10)) {
                    ::usleep(100); // 队列满时稍等再试
                }
                LOGD(kTag, "[child 生产] pushed: %d", i * 10);
                fflush(stdout); // _exit 不会刷新 stdio 缓冲，这里逐条刷新
            }
            // 父子进程并发运行，输出可能交错，没有固定的"分界点"，所以不画分隔线，
            // 而是由子进程自己报告"生产完毕"：无论和父进程的输出怎么交错，这句话都成立
            LOGD(kTag, "[child 生产] 全部 %d 个已生产完毕✅✅✅", kCount);
            fflush(stdout);
            _exit(0);
        } else {
            // 父进程（消费者）：与子进程并发地取数，直到收齐 kCount 个，然后再回收子进程。
            // 如果先 waitpid 再取，队列满时子进程等父进程取、父进程等子进程退出，会互相等待（死锁）。
            pid_t parentPid = getpid();
            LOGD(kTag, "parentPid2: %d", parentPid);
            LOGD(kTag, "[parent] child_pid: %d（父进程里 fork 返回的是子进程的 PID）", child_pid);
            int val = 0;
            int popped = 0;
            int status = 0;
            bool child_done = false;
            while (popped < kCount) {
                if (ring->pop(val)) {
                    LOGD(kTag, "[parent 消费] popped: %d", val);
                    ++popped;
                } else if (child_done) {
                    break; // 子进程已退出且队列已空，不会再有数据（子进程异常退出时防止死等）
                } else {
                    child_done = waitpid(child_pid, &status, WNOHANG) == child_pid;
                    usleep(100); // 队列空时稍等再试
                }
            }

            if (!child_done)
                ::waitpid(child_pid, &status, 0);
            LOGD(kTag, "[parent] total popped: %d items", popped);
        }
    }

    void Stage4SharedMem::run_all() {
        LOGI(kTag, "========================================");
        LOGI(kTag, "Stage 4: IPC Shared Memory");
        LOGI(kTag, "========================================");

        demo_fork_shared();
        demo_shm_open();
        demo_ring_buffer();

        LOGI(kTag, "===== Stage4: all demos complete =====");
    }

} // namespace mmap_learn
