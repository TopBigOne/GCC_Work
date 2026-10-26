/**
 * @file stage9_projects.h
 * @brief 阶段 9：综合实战 —— 用 mmap 实现两个小而完整的工具：MiniGrep 与 SimpleKVStore。
 *
 * 本阶段学什么：
 * - 只读映射 + madvise：像访问内存一样扫描文件（MiniGrep）
 * - 读写共享映射 + 倍数预分配扩容 + 重新映射 + 按页 msync 持久化（SimpleKVStore）
 * - 追加写（append-only）日志结构与"最后写入者胜出"的读取语义
 *
 * 对应的 mmap 知识点：
 * - MAP_PRIVATE + PROT_READ：零拷贝读，不会意外改到文件
 * - MAP_SHARED + PROT_WRITE：对内存的修改就是对文件的修改
 * - 文件变大后旧映射看不到新空间（访问超出原映射长度属于越界），必须先 ftruncate 再重新 mmap
 * - msync 决定数据何时真正落盘，是持久化正确性的关键
 *
 * @par 实际应用
 * - **ripgrep / ag / grep**：搜索大文件时 mmap 整个文件，用 memchr/SIMD 在内存上查找换行与模式。
 * - **LMDB**：整个数据库文件只读映射，读请求零拷贝直接返回指向映射内的指针。
 * - **LevelDB / RocksDB / Bitcask**：追加写日志（WAL / SSTable）+ 墓碑删除 + 后台合并，
 *   SimpleKVStore 就是这种日志结构存储最简化的雏形。
 * - **Kafka / RocketMQ**：消息日志文件与索引文件用 mmap 读写，配合 page cache 实现高吞吐。
 */
#pragma once

#include "../mmap_wrapper.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mmap_learn {

/**
 * @brief 基于 mmap 的简化版 grep：在文件中按行查找包含指定子串的行。
 *
 * 使用外观（Facade）模式：调用方只需 MiniGrep("fox").search_file(path)，
 * open / fstat / mmap / madvise / munmap / close 等细节都封装在内部。
 *
 * 实现思路：只读映射整个文件 → 用 string_view 直接指向映射内存切分每一行 →
 * 子串匹配，只有命中的行才拷贝成 std::string 返回。
 *
 * @par 实际应用
 * - 日志排查：在 GB 级日志里搜索 trace_id / 错误关键字，mmap 免去分块读取和拼接跨块行的麻烦。
 * - ripgrep 等工具对单个大文件就采用这种方式；对成千上万个小文件则改用 read()，
 *   因为每个文件 mmap/munmap 的固定开销会超过拷贝本身。
 *
 * @code
 * MiniGrep grep("ERROR");
 * for (const auto &m : grep.search_file("/var/log/app.log")) {
 *     printf("%d: %s\n", m.line_no, m.line.c_str());
 * }
 * @endcode
 */
class MiniGrep {
public:
    /** @brief 一条匹配结果。 */
    struct Match {
        int line_no;      ///< 行号，从 1 开始
        std::string line; ///< 匹配行的内容（不含换行符），已从映射中拷贝出来
    };

    /**
     * @brief 构造搜索器。
     * @param pattern 要查找的子串（普通字符串匹配，不支持正则）
     */
    explicit MiniGrep(const char *pattern);

    /**
     * @brief 映射文件并逐行查找 pattern。
     * @param filepath 要搜索的文件路径
     * @return 所有匹配行及其行号；空文件返回空列表
     * @throw std::runtime_error 打开或映射文件失败时抛出
     * @note 返回的 Match 已拷贝为 std::string，函数返回后映射被释放也不影响结果；
     *       若改成返回 string_view，则必须保证映射的生命周期长于结果。
     * @warning 搜索期间若文件被其他进程截断，访问已不存在的页会触发 SIGBUS。
     */
    std::vector<Match> search_file(const char *filepath) const;

private:
    std::string pattern_; ///< 要查找的子串
};

/**
 * @brief 基于 mmap 的追加写（append-only）持久化键值存储。
 *
 * 文件格式（小端序）：
 * @code
 * [Header]  magic(4) | version(4) | count(4) | data_offset(4) | data_end(4)   共 20 字节
 * [Entry]*  key_len(4) | val_len(4) | key(key_len) | value(val_len)
 * [空闲区]  data_end 之后到文件末尾，全部为 0，留给后续追加
 * @endcode
 *
 * - 写入（put）和删除（remove）都是在 data_end 处追加一条记录，从不原地修改旧数据；
 * - 删除记录是 val_len == kTombstone（0xFFFFFFFF）的"墓碑"（tombstone），没有 value 字节，
 *   因此空字符串是合法的值，不会被误当成删除；
 * - 读取时从 data_offset 扫描到 data_end，同一个 key 以最后一条为准（last-write-wins）；
 * - 文件容量按倍数预分配：只有空间不足时才 ftruncate 扩容并重新映射，
 *   所以文件大小 ≥ 实际数据量，数据末尾以文件头里的 data_end 为准；
 * - 使用 MAP_SHARED 映射，对内存的修改就是对文件的修改，每次写入只 msync 涉及到的页。
 *
 * @par 实际应用
 * 这是 Bitcask（Riak 的存储引擎）、LevelDB/RocksDB 的 WAL 与 SSTable、
 * Kafka 消息日志等"日志结构存储"的最简雏形：
 * - 追加写是顺序 I/O，速度快；崩溃时最多丢失尾部未写完的记录，旧数据不会损坏；
 * - 预分配 + 倍数扩容与 MMKV、Kafka 日志段的做法相同，避免每次写入都重新映射；
 * - 真实系统还会加：内存索引（key → 偏移，避免全表扫描）、校验和（CRC）、
 *   后台合并（compaction，清理被覆盖的旧值与墓碑）。
 *
 * @warning 仅用于学习：get/keys 都是 O(n) 全量扫描；不支持多线程或多进程并发写；
 *          data_end 是 32 位，单个文件最大 4GB。
 */
class SimpleKVStore {
    static constexpr uint32_t MAGIC = 0x4B56534D;      ///< 文件魔数，小端存储为字节 "MSVK"，用于识别文件格式
    static constexpr uint32_t VERSION = 2;             ///< 文件格式版本号；v2 增加 data_end 与显式墓碑标记
    static constexpr uint32_t HEADER_SIZE = 20;        ///< 文件头大小（字节）
    static constexpr uint32_t kTombstone = 0xFFFFFFFF; ///< val_len 取此保留值表示墓碑（删除标记）
    static constexpr size_t kInitialCapacity = 4096;   ///< 新建文件时预分配的容量（字节）

    /** @brief 文件头，位于文件开头，通过映射指针直接读写。 */
    struct Header {
        uint32_t magic;       ///< 魔数，必须等于 MAGIC
        uint32_t version;     ///< 格式版本，必须等于 VERSION
        uint32_t count;       ///< 记录总数（包含墓碑）
        uint32_t data_offset; ///< 第一条记录的字节偏移（当前恒为 HEADER_SIZE）
        uint32_t data_end;    ///< 最后一条记录之后的字节偏移，即下一次追加的位置
    };
    static_assert(sizeof(Header) == HEADER_SIZE, "Header layout must match HEADER_SIZE");

public:
    /**
     * @brief 打开已有的存储文件，不存在（或为空）则新建、预分配容量并写入文件头。
     * @param filepath 存储文件路径
     * @throw std::runtime_error 打开/创建失败、文件过小、文件头校验失败或 data_end 越界时抛出；
     *        fd 由 FileDescriptor 管理，抛异常时也会被自动关闭
     */
    explicit SimpleKVStore(const char *filepath);

    /** @brief msync 刷盘；随后映射与 fd 由各自的 RAII 成员自动释放。 */
    ~SimpleKVStore();

    /** @brief 禁止拷贝；声明了拷贝操作后移动操作也不会自动生成，因此同样不可移动。 */
    SimpleKVStore(const SimpleKVStore &) = delete;

    /** @brief 禁止拷贝赋值，避免两个对象共用同一个 fd 和映射而重复释放。 */
    SimpleKVStore &operator=(const SimpleKVStore &) = delete;

    /**
     * @brief 写入键值对：在 data_end 处追加一条记录，旧值不删除，读取时被新值覆盖。
     * @param key   键
     * @param value 值；可以为空字符串（与删除不同）
     * @throw std::runtime_error 扩容或重新映射失败时抛出
     */
    void put(std::string_view key, std::string_view value);

    /**
     * @brief 查找 key 的最新值。
     * @param key 键
     * @return 最后一条记录的值（可能是空字符串）；不存在或最后一条是墓碑时返回 std::nullopt
     * @note 全量扫描，复杂度 O(记录数)。真实系统会维护 key → 文件偏移的内存索引（如 Bitcask 的 keydir）。
     */
    std::optional<std::string> get(std::string_view key) const;

    /**
     * @brief 删除 key：追加一条 val_len == kTombstone 的墓碑记录，而不是真的抹掉旧数据。
     * @param key 要删除的键
     * @note 追加写系统的删除都是"标记删除"，旧数据要等合并（compaction）时才会被物理清理。
     */
    void remove(std::string_view key);

    /**
     * @brief 列出所有未被删除的 key（去重，按首次出现顺序）。
     * @return 当前有效的 key 列表
     * @note 用哈希表记录每个 key 在结果中的位置，整体 O(记录数)。
     */
    std::vector<std::string> keys() const;

private:
    std::string filepath_; ///< 存储文件路径
    FileDescriptor fd_;    ///< 存储文件的读写 fd（RAII，构造中途抛异常也会自动 close）
    MmapRegion region_;    ///< 整个文件的读写 MAP_SHARED 映射，长度等于当前文件容量
    size_t page_size_ = 0; ///< 系统页大小，msync 的起始地址必须按页对齐

    /**
     * @brief 以读写 + MAP_SHARED 映射整个文件，长度为当前文件大小（容量）。
     * @throw std::runtime_error fstat 或 mmap 失败时抛出
     */
    void map_file();

    /**
     * @brief 获取文件头指针（指向映射起始处）。
     * @return 可读写的 Header 指针
     */
    Header *header();

    /**
     * @brief 获取文件头指针（只读版本）。
     * @return 只读的 Header 指针
     */
    const Header *header() const;

    /**
     * @brief 用 MS_SYNC 把整个映射的修改同步写回磁盘。
     * @note 不调用 msync 时，脏页也会由内核择机写回；但进程崩溃不丢、断电会丢，
     *       需要"写成功即持久"的语义时必须 msync（数据库通常还会再 fsync 保证元数据落盘）。
     */
    void sync_map();

    /**
     * @brief 只把 [begin, end) 字节范围所在的页同步写回磁盘。
     *
     * msync 要求起始地址按页对齐，所以把 begin 向下取整到页边界。
     * 每次写入只同步新增记录所在的几页，而不是整个文件，写入成本不再随文件增大而增长。
     *
     * @param begin 起始字节偏移（相对映射起点）
     * @param end   结束字节偏移（不含）
     */
    void sync_range(size_t begin, size_t end);

    /**
     * @brief 确保容量至少为 required 字节，不足时按倍数扩容并重新映射。
     *
     * 新容量 = max(当前容量 × 2, required)，再向上取整到页大小。
     * 扩容步骤：ftruncate 扩大文件 → 重新 mmap 整个文件（旧映射由 MmapRegion 移动赋值时 munmap）。
     *
     * @param required 需要的最小文件大小（字节）
     * @throw std::runtime_error ftruncate 或 mmap 失败时抛出
     * @note 映射长度在 mmap 时就固定了，文件变大后旧映射看不到新增部分。
     *       Linux 可用 mremap 原地扩展映射，macOS 没有 mremap，只能 munmap + mmap，
     *       且新映射的地址可能变化，之前保存的指针全部失效。
     */
    void ensure_capacity(size_t required);

    /**
     * @brief 在 data_end 处追加一条记录，并更新文件头中的 count 与 data_end。
     *
     * 步骤：必要时扩容 → memcpy 写入记录 → msync 记录所在页 → 更新文件头 → msync 文件头。
     * 先落盘数据、再更新 data_end：即使中途崩溃，文件头也不会指向写了一半的记录。
     *
     * @param key     键
     * @param value   值（墓碑时忽略）
     * @param is_tombstone true 表示写入删除标记
     * @throw std::runtime_error 扩容失败时抛出
     */
    void append_entry(std::string_view key, std::string_view value, bool is_tombstone);

    /**
     * @brief 从 data_offset 到 data_end 依次遍历所有记录。
     *
     * key / value 以 string_view 形式直接指向映射内存（零拷贝），
     * 仅在 visitor 调用期间有效，下一次 put 触发扩容重新映射后会失效。
     * 遍历范围以文件头记录的 data_end 为准，不会读到预分配的 0 填充区；
     * 遇到长度越界的记录时停止遍历。
     *
     * @tparam Visitor 可调用对象，签名为 void(std::string_view key, std::string_view value, bool is_tombstone)
     * @param visitor 对每条记录调用一次；墓碑记录的 value 为空
     * @note 模板函数需要在头文件中给出完整实现，编译器才能在调用处实例化。
     */
    template <typename Visitor>
    void iterate_entries(Visitor visitor) const {
        if (!region_.valid()) return;

        const uint8_t *base = region_.as<const uint8_t>();
        const Header *hdr = header();
        size_t end = std::min<size_t>(hdr->data_end, region_.size());
        size_t offset = hdr->data_offset;

        while (offset + sizeof(uint32_t) * 2 <= end) {
            uint32_t key_len = 0;
            uint32_t val_len = 0;
            std::memcpy(&key_len, base + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            std::memcpy(&val_len, base + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            bool is_tombstone = (val_len == kTombstone);
            size_t value_bytes = is_tombstone ? 0 : val_len; // 墓碑没有 value 字节
            if (offset + key_len + value_bytes > end) break;  // 记录残缺或损坏，停止遍历

            std::string_view k(reinterpret_cast<const char *>(base + offset), key_len);
            offset += key_len;
            std::string_view v(reinterpret_cast<const char *>(base + offset), value_bytes);
            offset += value_bytes;

            visitor(k, v, is_tombstone);
        }
    }
};

/**
 * @brief 阶段 9 入口：运行 MiniGrep 与 SimpleKVStore 两个综合 demo。
 */
class Stage9Projects {
public:
    /**
     * @brief Demo 1：生成一个多行文本文件，用 MiniGrep 搜索 "fox"、"mmap" 和不存在的 "elephant"。
     *
     * @par 实际应用
     * 演示"把文件当成一段内存来处理"的编程模型：无需分块读取，
     * 行切分、子串查找都直接在映射内存上完成，这正是日志检索、文本搜索工具的核心思路。
     */
    static void demo_mini_grep();

    /**
     * @brief Demo 2：SimpleKVStore 的增删改查与持久化验证。
     *
     * 第一阶段写入 4 个键、覆盖 name、删除 level、写入空字符串值 note；
     * 第二阶段关闭后重新打开文件，验证数据仍在，覆盖、删除与空值的结果都被正确保留。
     *
     * @par 实际应用
     * 演示 mmap 做持久化存储的完整闭环：MAP_SHARED 写内存 → msync 落盘 → 重启后重新映射读取。
     * 配置存储、本地缓存、嵌入式数据库（LMDB、MMKV）都基于这一机制。
     */
    static void demo_kv_store();

    /** @brief 依次运行阶段 9 的全部 demo。 */
    static void run_all();
};

} // namespace mmap_learn
