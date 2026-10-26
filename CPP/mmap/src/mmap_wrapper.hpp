/**
 * @file mmap_wrapper.hpp
 * @brief 本项目所有阶段共用的 mmap 基础封装。
 *
 * - FileDescriptor : RAII 管理文件描述符（open / close）
 * - MmapRegion     : RAII 管理映射区域（mmap / munmap）
 * - MmapBuilder    : Builder 模式，链式配置 mmap 参数
 *
 * 资源在构造时获取、析构时释放；对象只能移动不能拷贝，
 * 保证同一个 fd / 映射在任意时刻只有一个所有者，避免重复释放。
 */
#pragma once

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace mmap_learn {

    /**
     * @brief 本项目所有临时文件统一存放的目录：程序运行时「当前工作目录」下的 mmap_tmp/。
     *
     * 不用系统的 /tmp：/tmp 是所有程序共用的，启动清理时容易误删别人的文件；
     * 放在专用子目录里，运行时能直接看到生成的文件，启动时也可以放心清空整个目录。
     * 由 main() 在启动时创建并清空。
     */
    inline constexpr const char *kWorkDir = "mmap_tmp";

    /**
     * @brief 拼出工作目录下某个文件的路径。
     * @param name 文件名，如 "mmap_learn_read.txt"
     * @return 相对路径，如 "mmap_tmp/mmap_learn_read.txt"
     */
    inline std::string work_path(const std::string &name) { return std::string(kWorkDir) + "/" + name; }

    /**
     * @brief RAII 文件描述符，构造时 open，析构时 close。
     *
     * mmap 建立映射后即使 close(fd)，映射依然有效（内核在 VMA 中持有了
     * file 的引用），因此 FileDescriptor 可以先于 MmapRegion 销毁。
     *
     * @code
     * FileDescriptor fd("hello.txt", O_RDWR);
     * off_t len = fd.size();
     * @endcode
     */
    class FileDescriptor {
        int fd_ = -1; ///< 持有的文件描述符，-1 表示不持有任何 fd

    public:
        /** @brief 构造一个不持有 fd 的空对象。 */
        FileDescriptor() = default;

        /**
         * @brief 打开文件并接管返回的 fd。
         * @param path  文件路径
         * @param flags open 标志，由"访问模式"按位或"可选标志"组成：
         *              - 访问模式（三选一）：O_RDONLY 只读、O_WRONLY 只写、O_RDWR 读写
         *              - O_CREAT：文件不存在时创建，权限取 mode
         *              - O_TRUNC：文件已存在时清空为 0 字节
         *              - O_APPEND：每次 write 都追加到文件末尾
         *              例：O_CREAT | O_RDWR | O_TRUNC 表示"没有就新建，有就清空，可读可写"
         * @param mode  新建文件的权限，仅在指定 O_CREAT 时生效，默认 0644
         * @throw std::runtime_error open 失败时抛出，消息中包含 strerror(errno)
         */
        explicit FileDescriptor(const char *path, int flags, mode_t mode = 0644) : fd_(::open(path, flags, mode)) {
            if (fd_ < 0) {
                throw std::runtime_error(std::string("open failed: ") + path + " - " + strerror(errno));
            }
        }

        /** @brief 若持有有效 fd 则关闭。 */
        ~FileDescriptor() {
            if (fd_ >= 0)
                ::close(fd_);
        }

        /** @brief 禁止拷贝，避免同一个 fd 被 close 两次。 */
        FileDescriptor(const FileDescriptor &) = delete;
        FileDescriptor &operator=(const FileDescriptor &) = delete;

        /**
         * @brief 移动构造，转移 fd 所有权。
         * @param other 源对象，移动后其 fd 置为 -1
         */
        FileDescriptor(FileDescriptor &&other) noexcept : fd_(other.fd_) {
            other.fd_ = -1;
        }

        /**
         * @brief 移动赋值，先关闭自身原有的 fd，再接管 other 的 fd。
         * @param other 源对象，移动后其 fd 置为 -1
         * @return 自身引用
         */
        FileDescriptor &operator=(FileDescriptor &&other) noexcept {
            if (this != &other) {
                if (fd_ >= 0)
                    ::close(fd_);
                fd_ = other.fd_;
                other.fd_ = -1;
            }
            return *this;
        }

        /**
         * @brief 获取原始 fd，用于传给 mmap 等系统调用，不转移所有权。
         * @return 文件描述符，无效时为 -1
         */
        int get() const { return fd_; }

        /**
         * @brief 是否持有有效 fd。
         * @return 持有时返回 true
         */
        bool valid() const { return fd_ >= 0; }

        /**
         * @brief 通过 fstat 获取文件大小，常用作 mmap 的 length。
         * @note 大小为 0 的文件不能直接 mmap（length 为 0 会返回 EINVAL）。
         * @return 文件字节数
         * @throw std::runtime_error fstat 失败时抛出
         */
        off_t size() const {
            struct stat st{};
            if (fstat(fd_, &st) < 0) {
                throw std::runtime_error(std::string("fstat failed: ") + strerror(errno));
            }
            return st.st_size;
        }

        /**
         * @brief 通过 ftruncate 设置文件大小，扩展部分填 0。
         * @note 写方式映射新文件前必须先扩展，否则访问超出文件末尾的页会触发 SIGBUS。
         * @param length 目标文件大小（字节）
         * @throw std::runtime_error ftruncate 失败时抛出
         */
        void truncate(off_t length) const {
            if (ftruncate(fd_, length) < 0) {
                throw std::runtime_error(std::string("ftruncate failed: ") + strerror(errno));
            }
        }
    };

    /**
     * @brief RAII 映射区域，构造时 mmap，析构时 munmap。
     *
     * 用 MAP_FAILED（即 (void*)-1）而不是 nullptr 表示无效映射，
     * 与 mmap 失败时的返回值保持一致。
     */
    class MmapRegion {
        void *mapped_addr_ = MAP_FAILED; ///< mmap 实际返回的映射起始地址，内核保证按页对齐；无效时为 MAP_FAILED
        size_t length_ = 0;              ///< 映射长度（字节），munmap 时需原样传回

    public:
        /** @brief 构造一个无效的空映射。 */
        MmapRegion() = default;

        /**
         * @brief 调用 mmap 建立映射，参数与 mmap(2) 一一对应。
         * @param hint_addr 建议的映射起始地址：告诉内核"最好把这段映射放在进程虚拟地址空间的这个位置"。
         *                  不带 MAP_FIXED 时内核可以不采纳，实际地址以 mmap 返回值（mapped_addr_）为准；
         *                  通常传 nullptr，表示不提建议，由内核自己挑一段空闲地址
         * @param length 映射长度，内核会向上取整到页大小
         * @param prot   访问权限：PROT_READ / PROT_WRITE / PROT_EXEC / PROT_NONE 的组合
         * @param flags  映射类型：MAP_SHARED 或 MAP_PRIVATE，可再组合 MAP_ANON 等
         * @param fd     被映射文件的描述符，匿名映射传 -1
         * @param offset 文件偏移，必须是页大小的整数倍
         * @throw std::runtime_error mmap 返回 MAP_FAILED 时抛出
         */
        MmapRegion(void *hint_addr, size_t length, int prot, int flags, int fd, off_t offset) : length_(length) {
            mapped_addr_ = ::mmap(hint_addr, length, prot, flags, fd, offset);
            if (mapped_addr_ == MAP_FAILED) {
                throw std::runtime_error(std::string("mmap failed: ") + strerror(errno));
            }
        }

        /**
         * @brief 若映射有效则 munmap。
         * @note munmap 不保证 MAP_SHARED 的脏页已落盘，需要时请先调用 sync()。
         */
        ~MmapRegion() {
            if (mapped_addr_ != MAP_FAILED) {
                ::munmap(mapped_addr_, length_);
            }
        }

        /** @brief 禁止拷贝，避免同一段映射被 munmap 两次。 */
        MmapRegion(const MmapRegion &) = delete;
        MmapRegion &operator=(const MmapRegion &) = delete;

        /**
         * @brief 移动构造，转移映射所有权。
         * @param other 源对象，移动后变为无效映射
         */
        MmapRegion(MmapRegion &&other) noexcept : mapped_addr_(other.mapped_addr_), length_(other.length_) {
            other.mapped_addr_ = MAP_FAILED;
            other.length_ = 0;
        }

        /**
         * @brief 移动赋值，先解除自身原有映射，再接管 other 的映射。
         * @param other 源对象，移动后变为无效映射
         * @return 自身引用
         */
        MmapRegion &operator=(MmapRegion &&other) noexcept {
            if (this != &other) {
                if (mapped_addr_ != MAP_FAILED)
                    ::munmap(mapped_addr_, length_);
                mapped_addr_ = other.mapped_addr_;
                length_ = other.length_;
                other.mapped_addr_ = MAP_FAILED;
                other.length_ = 0;
            }
            return *this;
        }

        /**
         * @brief 获取映射起始地址（可写版本），用于需要修改映射内容的场景。
         *
         * 非 const 对象调用时选中此重载，例如通过 MAP_SHARED 映射改写文件、
         * 向匿名映射或共享内存写入数据：
         * @code
         * MmapRegion region = MmapBuilder().length(len).readWrite().shared().file(fd.get()).build();
         * memcpy(region.data(), "hello", 5); // 修改会写回文件
         * @endcode
         *
         * @note 返回可写指针不代表一定能写，实际权限由 prot 决定；
         *       PROT_READ 映射上写入会触发 SIGSEGV。
         * @return 映射起始地址，无效时为 MAP_FAILED
         */
        void *data() { return mapped_addr_; }

        /**
         * @brief 获取映射起始地址（只读版本），用于只读取映射内容的场景。
         *
         * const 对象（如以 const MmapRegion & 传参）调用时选中此重载，
         * 返回 const 指针，编译期阻止通过它修改映射内容。适用于打印内容、
         * 计算校验和、搜索文本等不应改动数据的函数：
         * @code
         * void dump(const MmapRegion &r) {
         *     fwrite(r.data(), 1, r.size(), stdout); // 只能读，不能写
         * }
         * @endcode
         *
         * @return 映射起始地址的只读指针，无效时为 MAP_FAILED
         */
        const void *data() const { return mapped_addr_; }

        /**
         * @brief 获取映射长度。
         * @return 构造时传入的 length（字节）
         */
        size_t size() const { return length_; }

        /**
         * @brief 是否持有有效映射。
         * @return 持有时返回 true
         */
        bool valid() const { return mapped_addr_ != MAP_FAILED; }

        /**
         * @brief 将映射区域按 T 类型指针访问，如 as<char>()、as<Header>()。
         * @tparam T 目标类型，调用方需保证其大小与对齐不超出映射范围
         * @return 指向映射起始地址的 T 指针
         */
        template<typename T>
        T *as() {
            return static_cast<T *>(mapped_addr_);
        }

        /** @copydoc as() */
        template<typename T>
        const T *as() const {
            return static_cast<const T *>(mapped_addr_);
        }

        /**
         * @brief 调用 msync，把 MAP_SHARED 映射中的修改写回文件。
         * @param flags MS_SYNC 阻塞直到写完；MS_ASYNC 只发起写回立即返回
         * @return 成功返回 0，失败返回 -1 并设置 errno
         */
        int sync(int flags = MS_SYNC) { return ::msync(mapped_addr_, length_, flags); }

        /**
         * @brief 调用 mprotect，运行时修改整个映射的页权限。
         * @param prot 新权限，如改为 PROT_READ 后再写入会触发 SIGSEGV
         * @return 成功返回 0，失败返回 -1 并设置 errno
         */
        int protect(int prot) { return ::mprotect(mapped_addr_, length_, prot); }

        /**
         * @brief 调用 madvise，提示内核访问模式以优化预读与回收。
         * @param advice MADV_SEQUENTIAL / MADV_RANDOM / MADV_WILLNEED / MADV_DONTNEED 等
         * @return 成功返回 0，失败返回 -1 并设置 errno
         */
        int advise(int advice) { return ::madvise(mapped_addr_, length_, advice); }

        /**
         * @brief 调用 mlock，锁定物理页防止被换出，受 RLIMIT_MEMLOCK 限制。
         * @return 成功返回 0，失败返回 -1 并设置 errno
         */
        int lock() { return ::mlock(mapped_addr_, length_); }

        /**
         * @brief 调用 munlock，解除 lock() 的锁定。
         * @return 成功返回 0，失败返回 -1 并设置 errno
         */
        int unlock() { return ::munlock(mapped_addr_, length_); }
    };

    /**
     * @brief 以链式调用配置 mmap 参数并构建 MmapRegion。
     *
     * 默认配置：只读、MAP_PRIVATE、偏移 0、由内核选择地址。
     *
     * @code
     * FileDescriptor fd("hello.txt", O_RDWR);
     * MmapRegion file_map = MmapBuilder().length(fd.size()).readWrite().shared().file(fd.get()).build();
     * MmapRegion anon_map = MmapBuilder().length(4096).readWrite().priv().anonymous().build();
     * @endcode
     *
     * shared() / priv() 只切换共享/私有位，保留 MAP_ANON 等其他标志，
     * 因此与 anonymous() 的调用顺序无关。
     *
     * @warning flags() 会整体覆盖所有标志（包括 anonymous() 设置的 MAP_ANON）。
     */
    class MmapBuilder {
        void *hint_addr_ = nullptr; ///< 建议的映射起始地址（内核可不采纳）；nullptr 表示不提建议
        size_t length_ = 0;        ///< 映射长度（字节），build() 时必须大于 0
        int prot_ = PROT_READ;     ///< 访问权限
        int flags_ = MAP_PRIVATE;  ///< 映射类型标志
        int fd_ = -1;              ///< 被映射文件的描述符，匿名映射为 -1
        off_t offset_ = 0;         ///< 文件偏移，必须按页对齐

    public:
        /**
         * @brief 设置建议的映射起始地址：不带 MAP_FIXED 时内核可以不采纳，带 MAP_FIXED 时必须放在这里。
         * @param hint_addr 建议的映射起始地址（应按页对齐）
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &address(void *hint_addr) {
            hint_addr_ = hint_addr;
            return *this;
        }

        /**
         * @brief 设置映射长度。
         * @param length 映射长度（字节），内核会向上取整到页大小
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &length(size_t length) {
            length_ = length;
            return *this;
        }

        /**
         * @brief 直接设置访问权限，会覆盖 readOnly() / readWrite() 的结果。
         * @param prot 访问权限，PROT_READ / PROT_WRITE / PROT_EXEC / PROT_NONE 的组合
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &protection(int prot) {
            prot_ = prot;
            return *this;
        }

        /**
         * @brief 直接设置映射标志，会覆盖之前所有 flags（包括 anonymous()）。
         * @param flags 映射标志，MAP_SHARED / MAP_PRIVATE / MAP_ANON 等的组合
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &flags(int flags) {
            flags_ = flags;
            return *this;
        }

        /**
         * @brief 设置被映射文件的描述符。
         * @param fd 文件描述符，所有权仍归调用方
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &file(int fd) {
            fd_ = fd;
            return *this;
        }

        /**
         * @brief 设置文件偏移。
         * @param offset 文件偏移（字节），必须是 sysconf(_SC_PAGESIZE) 的整数倍，否则 mmap 返回 EINVAL
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &offset(off_t offset) {
            offset_ = offset;
            return *this;
        }

        /**
         * @brief 设置为只读（PROT_READ）。
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &readOnly() {
            prot_ = PROT_READ;
            return *this;
        }

        /**
         * @brief 设置为可读写（PROT_READ | PROT_WRITE）。
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &readWrite() {
            prot_ = PROT_READ | PROT_WRITE;
            return *this;
        }

        /**
         * @brief 设置为共享映射（MAP_SHARED），修改写回文件且进程间可见。
         * @note 只替换共享/私有位，MAP_ANON 等其他标志保持不变。
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &shared() {
            flags_ = (flags_ & ~(MAP_SHARED | MAP_PRIVATE)) | MAP_SHARED;
            return *this;
        }

        /**
         * @brief 设置为私有映射（MAP_PRIVATE），写时复制，修改不影响文件。
         * @note 只替换共享/私有位，MAP_ANON 等其他标志保持不变。
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &priv() {
            flags_ = (flags_ & ~(MAP_SHARED | MAP_PRIVATE)) | MAP_PRIVATE;
            return *this;
        }

        /**
         * @brief 追加 MAP_ANON 标志，并将 fd 置为 -1，映射内容初始为 0。
         * @note MAP_ANON 与 MAP_ANONYMOUS 等价，macOS 上前者兼容性更好。
         * @return 自身引用，用于链式调用
         */
        MmapBuilder &anonymous() {
            flags_ |= MAP_ANON;
            fd_ = -1;
            return *this;
        }

        /**
         * @brief 按当前配置创建映射。
         * @return 持有新映射的 MmapRegion
         * @throw std::invalid_argument length 为 0 时抛出
         * @throw std::runtime_error    mmap 失败时由 MmapRegion 构造函数抛出
         */
        MmapRegion build() {
            if (length_ == 0) {
                throw std::invalid_argument("MmapBuilder: length must be > 0");
            }
            return MmapRegion(hint_addr_, length_, prot_, flags_, fd_, offset_);
        }
    };

} // namespace mmap_learn
