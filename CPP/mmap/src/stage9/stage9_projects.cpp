#include "stage9_projects.h"

#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <stdexcept>
#include <sys/mman.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>

namespace mmap_learn {

// ============================================================
// MiniGrep
// ============================================================

MiniGrep::MiniGrep(const char *pattern) : pattern_(pattern) {}

std::vector<MiniGrep::Match> MiniGrep::search_file(const char *filepath) const {
    std::vector<Match> results;

    // 通过 RAII 封装以只读方式打开文件
    FileDescriptor fd(filepath, O_RDONLY);
    off_t file_size = fd.size();
    if (file_size == 0) {
        return results; // 空文件不能 mmap（length 为 0 返回 EINVAL），直接返回
    }

    // 只读 + 私有映射整个文件：零拷贝读取，且不可能意外修改文件
    MmapRegion region = MmapBuilder()
                            .length(static_cast<size_t>(file_size))
                            .readOnly()
                            .priv()
                            .file(fd.get())
                            .offset(0)
                            .build();

    // 告诉内核将顺序读取：内核会加大预读窗口，并更积极地回收已读过的页
    region.advise(MADV_SEQUENTIAL);

    const char *data = region.as<const char>();
    size_t len = region.size();

    // 逐行扫描：整个文件就是一段连续内存，不需要处理"一行跨两个读缓冲区"的问题
    int line_no = 0;
    size_t pos = 0;
    while (pos < len) {
        ++line_no;

        // 找到当前行的结尾（'\n' 或文件末尾）
        size_t eol = pos;
        while (eol < len && data[eol] != '\n') {
            ++eol;
        }

        // string_view 直接指向映射内存，不拷贝（不含结尾换行符）
        std::string_view line_view(data + pos, eol - pos);

        // 子串匹配，命中才拷贝出来
        if (line_view.find(pattern_) != std::string_view::npos) {
            results.push_back({line_no, std::string(line_view)});
        }

        pos = eol + 1; // 跳过换行符
    }

    return results;
    // 离开作用域：region 析构 munmap，fd 析构 close
}

// ============================================================
// SimpleKVStore
// ============================================================

SimpleKVStore::SimpleKVStore(const char *filepath)
    : filepath_(filepath),
      // 不存在就创建；FileDescriptor 接管 fd，后面任何一步抛异常都会自动 close
      fd_(filepath, O_RDWR | O_CREAT, 0644),
      page_size_(static_cast<size_t>(::sysconf(_SC_PAGESIZE))) {
    off_t size = fd_.size();

    if (size == 0) {
        // 新文件（或空文件）：先预分配容量，否则映射后写文件头会触发 SIGBUS
        fd_.truncate(static_cast<off_t>(kInitialCapacity));
        map_file();

        // 直接通过映射指针初始化文件头，写内存即写文件
        Header *hdr = header();
        hdr->magic = MAGIC;
        hdr->version = VERSION;
        hdr->count = 0;
        hdr->data_offset = HEADER_SIZE;
        hdr->data_end = HEADER_SIZE;

        sync_range(0, HEADER_SIZE);
    } else {
        if (static_cast<size_t>(size) < HEADER_SIZE) {
            throw std::runtime_error("SimpleKVStore: file too small to contain header");
        }
        map_file();

        // 校验魔数和版本号，防止把其他格式的文件当成 KV 存储解析
        const Header *hdr = header();
        if (hdr->magic != MAGIC || hdr->version != VERSION) {
            throw std::runtime_error("SimpleKVStore: invalid file header");
        }
        // data_end 必须落在文件范围内，否则文件已损坏
        if (hdr->data_offset < HEADER_SIZE || hdr->data_end < hdr->data_offset ||
            hdr->data_end > region_.size()) {
            throw std::runtime_error("SimpleKVStore: corrupted data range in header");
        }
    }
}

SimpleKVStore::~SimpleKVStore() {
    // 每次写入都已按页 msync，这里再整体同步一次兜底；
    // 之后 region_ 析构 munmap，fd_ 析构 close
    sync_map();
}

void SimpleKVStore::put(std::string_view key, std::string_view value) {
    append_entry(key, value, false);
}

std::optional<std::string> SimpleKVStore::get(std::string_view key) const {
    std::optional<std::string> result;
    iterate_entries([&](std::string_view k, std::string_view v, bool is_tombstone) {
        if (k == key) {
            if (is_tombstone) {
                result = std::nullopt;
            } else {
                result = std::string(v); // 空字符串也是合法值
            }
        }
    });
    return result;
}

void SimpleKVStore::remove(std::string_view key) {
    append_entry(key, std::string_view{}, true); // val_len = kTombstone 即墓碑
}

std::vector<std::string> SimpleKVStore::keys() const {
    // 记录每个 key 的最终状态（true = 有效，false = 已删除），保持首次出现顺序
    std::vector<std::pair<std::string, bool>> seen_order;
    // key → 在 seen_order 中的下标，查找 O(1)，整体 O(n)
    std::unordered_map<std::string, size_t> index;
    iterate_entries([&](std::string_view k, std::string_view /*v*/, bool is_tombstone) {
        auto it = index.find(std::string(k));
        if (it != index.end()) {
            seen_order[it->second].second = !is_tombstone;
        } else {
            index.emplace(std::string(k), seen_order.size());
            seen_order.push_back({std::string(k), !is_tombstone});
        }
    });

    std::vector<std::string> result;
    for (const auto &p : seen_order) {
        if (p.second) {
            result.push_back(p.first);
        }
    }
    return result;
}

void SimpleKVStore::map_file() {
    // 映射整个文件（长度 = 当前容量）；移动赋值会先 munmap 旧映射
    size_t size = static_cast<size_t>(fd_.size());
    region_ = MmapBuilder()
                  .length(size)
                  .readWrite()
                  .shared()
                  .file(fd_.get())
                  .build();
}

SimpleKVStore::Header *SimpleKVStore::header() {
    return region_.as<Header>();
}

const SimpleKVStore::Header *SimpleKVStore::header() const {
    return region_.as<const Header>();
}

void SimpleKVStore::sync_map() {
    if (region_.valid()) {
        region_.sync(MS_SYNC);
    }
}

void SimpleKVStore::sync_range(size_t begin, size_t end) {
    if (!region_.valid() || begin >= end) return;
    // msync 的地址必须按页对齐：起点向下取整到页边界
    size_t aligned = begin / page_size_ * page_size_;
    ::msync(region_.as<uint8_t>() + aligned, end - aligned, MS_SYNC);
}

void SimpleKVStore::ensure_capacity(size_t required) {
    size_t capacity = region_.size();
    if (required <= capacity) return; // 容量够用，不需要扩容和重新映射

    // 倍数增长，摊还后每次写入的扩容成本是 O(1)；再向上取整到页大小
    size_t new_capacity = std::max(capacity * 2, required);
    new_capacity = (new_capacity + page_size_ - 1) / page_size_ * page_size_;

    // 先扩展文件，否则写入超出文件末尾的映射区域会触发 SIGBUS
    fd_.truncate(static_cast<off_t>(new_capacity));

    // 重新映射，让映射覆盖新增的空间（旧指针全部失效）
    map_file();
}

void SimpleKVStore::append_entry(std::string_view key, std::string_view value, bool is_tombstone) {
    uint32_t key_len = static_cast<uint32_t>(key.size());
    uint32_t val_len = is_tombstone ? kTombstone : static_cast<uint32_t>(value.size());
    size_t value_bytes = is_tombstone ? 0 : value.size();
    size_t entry_size = sizeof(uint32_t) * 2 + key_len + value_bytes;

    size_t start = header()->data_end;
    size_t new_end = start + entry_size;
    if (new_end > UINT32_MAX) {
        throw std::runtime_error("SimpleKVStore: file would exceed 4GB limit");
    }
    ensure_capacity(new_end);

    // 在 data_end 处写入记录；用 memcpy 写整数，避免未对齐访问
    uint8_t *ptr = region_.as<uint8_t>() + start;
    std::memcpy(ptr, &key_len, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    std::memcpy(ptr, &val_len, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    std::memcpy(ptr, key.data(), key_len);
    ptr += key_len;
    if (value_bytes > 0) {
        std::memcpy(ptr, value.data(), value_bytes);
    }

    // 先让记录本身落盘（只同步它所在的页）……
    sync_range(start, new_end);

    // ……再更新文件头：崩溃时 data_end 不会指向写了一半的记录
    Header *hdr = header();
    hdr->count += 1;
    hdr->data_end = static_cast<uint32_t>(new_end);
    sync_range(0, HEADER_SIZE);
}

// ============================================================
// Stage9Projects
// ============================================================

void Stage9Projects::demo_mini_grep() {
    printf("=== Stage 9: MiniGrep Demo ===\n\n");

    const std::string tmp_path_str = work_path("mmap_learn_grep_input.txt");
    const char *tmp_path = tmp_path_str.c_str();

    // 用标准 C 文件 I/O 创建多行测试文本
    {
        FILE *f = std::fopen(tmp_path, "w");
        if (!f) {
            printf("[ERROR] Cannot create temp file %s\n", tmp_path);
            return;
        }
        std::fprintf(f,
                     "The quick brown fox jumps over the lazy dog\n"
                     "Memory-mapped I/O is efficient for large files\n"
                     "mmap provides zero-copy access to file data\n"
                     "The fox was not really that quick\n"
                     "Virtual memory makes mmap possible\n"
                     "Page faults drive demand paging for mmap regions\n"
                     "A second fox appeared near the fence\n"
                     "System calls like mmap are fundamental to Unix\n");
        std::fclose(f);
    }

    // 搜索 "fox"
    MiniGrep grep("fox");
    auto matches = grep.search_file(tmp_path);

    printf("Pattern: \"fox\"\n");
    printf("File:    %s\n", tmp_path);
    printf("Matches: %zu\n\n", matches.size());
    for (const auto &m : matches) {
        printf("  Line %d: %s\n", m.line_no, m.line.c_str());
    }

    // 搜索 "mmap"
    MiniGrep grep2("mmap");
    auto matches2 = grep2.search_file(tmp_path);

    printf("\nPattern: \"mmap\"\n");
    printf("Matches: %zu\n\n", matches2.size());
    for (const auto &m : matches2) {
        printf("  Line %d: %s\n", m.line_no, m.line.c_str());
    }

    // 搜索不存在的内容，验证结果为空
    MiniGrep grep3("elephant");
    auto matches3 = grep3.search_file(tmp_path);
    printf("\nPattern: \"elephant\"\n");
    printf("Matches: %zu (expected 0)\n", matches3.size());

    // 清理临时文件
    ::unlink(tmp_path);
    printf("\n");
}

void Stage9Projects::demo_kv_store() {
    printf("=== Stage 9: SimpleKVStore Demo ===\n\n");

    const std::string db_path_str = work_path("mmap_learn_kvstore.db");
    const char *db_path = db_path_str.c_str();

    // 删除上次运行可能残留的文件，保证从空库开始
    ::unlink(db_path);

    // 阶段 1：创建存储并写入数据
    {
        SimpleKVStore store(db_path);

        store.put("name", "Alice");
        store.put("language", "C++");
        store.put("topic", "mmap");
        store.put("level", "intermediate");
        printf("[put] Inserted 4 key-value pairs\n");

        // 读回写入的值
        auto v1 = store.get("name");
        auto v2 = store.get("language");
        auto v3 = store.get("topic");
        printf("[get] name     = %s\n", v1.value_or("(null)").c_str());
        printf("[get] language = %s\n", v2.value_or("(null)").c_str());
        printf("[get] topic    = %s\n", v3.value_or("(null)").c_str());

        // 覆盖已有的 key：追加一条新记录，读取时新值胜出
        store.put("name", "Bob");
        auto v4 = store.get("name");
        printf("[get] name (after overwrite) = %s\n", v4.value_or("(null)").c_str());

        // 删除 key：追加墓碑记录
        store.remove("level");
        auto v5 = store.get("level");
        printf("[get] level (after remove) = %s (expected null)\n", v5.value_or("(null)").c_str());

        // 空字符串是合法值，不会被当成墓碑
        store.put("note", "");
        auto v6 = store.get("note");
        printf("[get] note (empty value) = \"%s\" (expected \"\", not null)\n", v6 ? v6->c_str() : "(null)");

        // 列出当前有效的 key
        auto k = store.keys();
        printf("[keys] Active keys (%zu): ", k.size());
        for (size_t i = 0; i < k.size(); ++i) {
            printf("%s%s", k[i].c_str(), (i + 1 < k.size()) ? ", " : "");
        }
        printf("\n\n");
    }
    // store 析构：msync，随后 region_ munmap、fd_ close

    // 阶段 2：重新打开文件，验证数据已持久化
    {
        printf("[reopen] Verifying persistence after close/reopen...\n");
        SimpleKVStore store2(db_path);

        auto v1 = store2.get("name");
        auto v2 = store2.get("language");
        auto v3 = store2.get("topic");
        auto v4 = store2.get("level");
        auto v5 = store2.get("note");

        printf("[get] name     = %s (expected Bob)\n", v1.value_or("(null)").c_str());
        printf("[get] language = %s (expected C++)\n", v2.value_or("(null)").c_str());
        printf("[get] topic    = %s (expected mmap)\n", v3.value_or("(null)").c_str());
        printf("[get] level    = %s (expected null - was deleted)\n", v4.value_or("(null)").c_str());
        printf("[get] note     = \"%s\" (expected \"\" - empty value kept)\n", v5 ? v5->c_str() : "(null)");

        auto k = store2.keys();
        printf("[keys] Active keys (%zu): ", k.size());
        for (size_t i = 0; i < k.size(); ++i) {
            printf("%s%s", k[i].c_str(), (i + 1 < k.size()) ? ", " : "");
        }
        printf("\n");
    }

    // 清理存储文件
    ::unlink(db_path);
    printf("\n");
}

void Stage9Projects::run_all() {
    printf("############################################################\n");
    printf("# Stage 9: Comprehensive Practical Projects                #\n");
    printf("############################################################\n\n");
    demo_mini_grep();
    demo_kv_store();
    printf("############################################################\n");
    printf("# Stage 9: All demos complete                              #\n");
    printf("############################################################\n");
}

} // namespace mmap_learn
