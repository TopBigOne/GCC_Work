#pragma once

#include "mmap_wrapper.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mmap_learn {

// ============================================================
// MiniGrep - A grep-like tool using mmap (Facade pattern)
//
// Memory-maps the target file read-only for zero-copy scanning,
// then iterates line-by-line looking for substring matches.
// ============================================================
class MiniGrep {
public:
    struct Match {
        int line_no;
        std::string line;
    };

    explicit MiniGrep(const char *pattern) : pattern_(pattern) {}

    /// mmap the file and scan every line for the pattern.
    /// Returns all matching lines with their 1-based line numbers.
    std::vector<Match> search_file(const char *filepath) const {
        std::vector<Match> results;

        // Open read-only via RAII wrapper
        FileDescriptor fd(filepath, O_RDONLY);
        off_t file_size = fd.size();
        if (file_size == 0) {
            return results; // empty file, nothing to search
        }

        // Memory-map the entire file read-only + private (zero-copy read)
        MmapRegion region = MmapBuilder()
                                .length(static_cast<size_t>(file_size))
                                .readOnly()
                                .priv()
                                .file(fd.get())
                                .offset(0)
                                .build();

        // Hint the OS we will read sequentially
        region.advise(MADV_SEQUENTIAL);

        const char *data = region.as<const char>();
        size_t len = region.size();

        // Scan line by line
        int line_no = 0;
        size_t pos = 0;
        while (pos < len) {
            ++line_no;

            // Find end of current line
            size_t eol = pos;
            while (eol < len && data[eol] != '\n') {
                ++eol;
            }

            // Extract the line content (without trailing newline)
            std::string_view line_view(data + pos, eol - pos);

            // Substring match
            if (line_view.find(pattern_) != std::string_view::npos) {
                results.push_back({line_no, std::string(line_view)});
            }

            pos = eol + 1; // skip past newline
        }

        return results;
    }

private:
    std::string pattern_;
};

// ============================================================
// SimpleKVStore - Persistent key-value store using mmap
//
// On-disk format:
//   [Header]  magic(4) | version(4) | count(4) | data_offset(4) = 16 bytes
//   [Entry]*  key_len(4) | val_len(4) | key(key_len) | value(val_len)
//
// A tombstone entry has val_len == 0.
// Reads scan all entries and the *last* entry for a key wins.
// Uses MAP_SHARED so writes are visible to the OS / persist.
// ============================================================
class SimpleKVStore {
    static constexpr uint32_t MAGIC = 0x4B56534D;   // "MSVK" little-endian
    static constexpr uint32_t VERSION = 1;
    static constexpr uint32_t HEADER_SIZE = 16;

    struct Header {
        uint32_t magic;
        uint32_t version;
        uint32_t count;       // total entry count (including tombstones)
        uint32_t data_offset; // byte offset where first entry begins
    };

public:
    explicit SimpleKVStore(const char *filepath) : filepath_(filepath) {
        // Try to open existing file first; create if missing.
        bool created = false;
        int open_flags = O_RDWR;

        int raw_fd = ::open(filepath, open_flags);
        if (raw_fd < 0) {
            // File does not exist -- create it.
            raw_fd = ::open(filepath, O_RDWR | O_CREAT, 0644);
            if (raw_fd < 0) {
                throw std::runtime_error(std::string("SimpleKVStore: cannot open ") + filepath + " - " +
                                         strerror(errno));
            }
            created = true;
        }

        fd_ = FileDescriptor(); // default
        // We need to manage fd manually because FileDescriptor ctor would re-open.
        // Instead, wrap the raw fd using a lightweight holder.
        fd_raw_ = raw_fd;

        if (created) {
            // Write the header
            if (::ftruncate(fd_raw_, HEADER_SIZE) < 0) {
                ::close(fd_raw_);
                throw std::runtime_error("SimpleKVStore: ftruncate failed");
            }
            file_size_ = HEADER_SIZE;
            map_file();

            // Initialize header
            Header *hdr = reinterpret_cast<Header *>(mapped_addr_);
            hdr->magic = MAGIC;
            hdr->version = VERSION;
            hdr->count = 0;
            hdr->data_offset = HEADER_SIZE;

            sync_map();
        } else {
            struct stat st {};
            ::fstat(fd_raw_, &st);
            file_size_ = static_cast<size_t>(st.st_size);
            if (file_size_ < HEADER_SIZE) {
                ::close(fd_raw_);
                throw std::runtime_error("SimpleKVStore: file too small to contain header");
            }
            map_file();

            // Validate header
            const Header *hdr = reinterpret_cast<const Header *>(mapped_addr_);
            if (hdr->magic != MAGIC || hdr->version != VERSION) {
                unmap_file();
                ::close(fd_raw_);
                throw std::runtime_error("SimpleKVStore: invalid file header");
            }
        }
    }

    ~SimpleKVStore() {
        unmap_file();
        if (fd_raw_ >= 0) {
            ::close(fd_raw_);
        }
    }

    // Non-copyable, non-movable (simplicity)
    SimpleKVStore(const SimpleKVStore &) = delete;
    SimpleKVStore &operator=(const SimpleKVStore &) = delete;

    /// Append a key-value entry. Extends the file and remaps.
    void put(std::string_view key, std::string_view value) {
        append_entry(key, value);
    }

    /// Scan all entries for the latest value associated with key.
    /// Returns std::nullopt if not found or deleted (tombstone).
    std::optional<std::string> get(std::string_view key) const {
        std::optional<std::string> result;
        iterate_entries([&](std::string_view k, std::string_view v, bool is_tombstone) {
            if (k == key) {
                if (is_tombstone) {
                    result = std::nullopt;
                } else {
                    result = std::string(v);
                }
            }
        });
        return result;
    }

    /// Append a tombstone entry (val_len == 0) for the given key.
    void remove(std::string_view key) {
        append_entry(key, std::string_view{}); // val_len = 0 => tombstone
    }

    /// List all non-deleted keys (unique, last-write-wins).
    std::vector<std::string> keys() const {
        // Track last state for each key (true = alive, false = tombstoned)
        std::vector<std::pair<std::string, bool>> seen_order;
        // We use a simple linear scan; for a production DB you would index.
        iterate_entries([&](std::string_view k, std::string_view /*v*/, bool is_tombstone) {
            bool found = false;
            for (auto &p : seen_order) {
                if (p.first == k) {
                    p.second = !is_tombstone;
                    found = true;
                    break;
                }
            }
            if (!found) {
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

private:
    std::string filepath_;
    FileDescriptor fd_;   // unused placeholder (we manage fd_raw_ directly)
    int fd_raw_ = -1;
    void *mapped_addr_ = MAP_FAILED;
    size_t file_size_ = 0;

    void map_file() {
        if (file_size_ == 0) return;
        mapped_addr_ = ::mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_raw_, 0);
        if (mapped_addr_ == MAP_FAILED) {
            throw std::runtime_error(std::string("SimpleKVStore mmap failed: ") + strerror(errno));
        }
    }

    void unmap_file() {
        if (mapped_addr_ != MAP_FAILED && file_size_ > 0) {
            ::msync(mapped_addr_, file_size_, MS_SYNC);
            ::munmap(mapped_addr_, file_size_);
            mapped_addr_ = MAP_FAILED;
        }
    }

    void sync_map() {
        if (mapped_addr_ != MAP_FAILED && file_size_ > 0) {
            ::msync(mapped_addr_, file_size_, MS_SYNC);
        }
    }

    /// Remap the file after it has grown.
    void remap() {
        unmap_file();
        struct stat st {};
        ::fstat(fd_raw_, &st);
        file_size_ = static_cast<size_t>(st.st_size);
        map_file();
    }

    /// Append an entry to the file. If value is empty, it is a tombstone.
    void append_entry(std::string_view key, std::string_view value) {
        uint32_t key_len = static_cast<uint32_t>(key.size());
        uint32_t val_len = static_cast<uint32_t>(value.size());
        size_t entry_size = sizeof(uint32_t) * 2 + key_len + val_len;

        // Grow the file
        size_t new_size = file_size_ + entry_size;
        if (::ftruncate(fd_raw_, static_cast<off_t>(new_size)) < 0) {
            throw std::runtime_error(std::string("SimpleKVStore ftruncate failed: ") + strerror(errno));
        }

        // Remap to see the new space
        remap();

        // Write entry at old end
        uint8_t *ptr = reinterpret_cast<uint8_t *>(mapped_addr_) + (new_size - entry_size);
        std::memcpy(ptr, &key_len, sizeof(uint32_t));
        ptr += sizeof(uint32_t);
        std::memcpy(ptr, &val_len, sizeof(uint32_t));
        ptr += sizeof(uint32_t);
        std::memcpy(ptr, key.data(), key_len);
        ptr += key_len;
        if (val_len > 0) {
            std::memcpy(ptr, value.data(), val_len);
        }

        // Update header count
        Header *hdr = reinterpret_cast<Header *>(mapped_addr_);
        hdr->count += 1;

        sync_map();
    }

    /// Iterate all entries, calling visitor(key, value, is_tombstone) for each.
    template <typename Visitor>
    void iterate_entries(Visitor visitor) const {
        if (mapped_addr_ == MAP_FAILED || file_size_ <= HEADER_SIZE) return;

        const uint8_t *base = reinterpret_cast<const uint8_t *>(mapped_addr_);
        size_t offset = HEADER_SIZE;

        while (offset + sizeof(uint32_t) * 2 <= file_size_) {
            uint32_t key_len = 0;
            uint32_t val_len = 0;
            std::memcpy(&key_len, base + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            std::memcpy(&val_len, base + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);

            if (offset + key_len + val_len > file_size_) break; // corrupt / partial

            std::string_view k(reinterpret_cast<const char *>(base + offset), key_len);
            offset += key_len;
            std::string_view v(reinterpret_cast<const char *>(base + offset), val_len);
            offset += val_len;

            visitor(k, v, val_len == 0);
        }
    }
};

// ============================================================
// Stage9Projects - Static demo entry points
// ============================================================
class Stage9Projects {
public:
    static void demo_mini_grep() {
        printf("=== Stage 9: MiniGrep Demo ===\n\n");

        const char *tmp_path = "/tmp/mmap_learn_grep_input.txt";

        // Create a temp file with multi-line text
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

        // Search for "fox"
        MiniGrep grep("fox");
        auto matches = grep.search_file(tmp_path);

        printf("Pattern: \"fox\"\n");
        printf("File:    %s\n", tmp_path);
        printf("Matches: %zu\n\n", matches.size());
        for (const auto &m : matches) {
            printf("  Line %d: %s\n", m.line_no, m.line.c_str());
        }

        // Search for "mmap"
        MiniGrep grep2("mmap");
        auto matches2 = grep2.search_file(tmp_path);

        printf("\nPattern: \"mmap\"\n");
        printf("Matches: %zu\n\n", matches2.size());
        for (const auto &m : matches2) {
            printf("  Line %d: %s\n", m.line_no, m.line.c_str());
        }

        // Search for something that does not exist
        MiniGrep grep3("elephant");
        auto matches3 = grep3.search_file(tmp_path);
        printf("\nPattern: \"elephant\"\n");
        printf("Matches: %zu (expected 0)\n", matches3.size());

        // Cleanup
        ::unlink(tmp_path);
        printf("\n");
    }

    static void demo_kv_store() {
        printf("=== Stage 9: SimpleKVStore Demo ===\n\n");

        const char *db_path = "/tmp/mmap_learn_kvstore.db";

        // Remove any leftover file from prior run
        ::unlink(db_path);

        // Phase 1: Create store and write data
        {
            SimpleKVStore store(db_path);

            store.put("name", "Alice");
            store.put("language", "C++");
            store.put("topic", "mmap");
            store.put("level", "intermediate");
            printf("[put] Inserted 4 key-value pairs\n");

            // Read them back
            auto v1 = store.get("name");
            auto v2 = store.get("language");
            auto v3 = store.get("topic");
            printf("[get] name     = %s\n", v1.value_or("(null)").c_str());
            printf("[get] language = %s\n", v2.value_or("(null)").c_str());
            printf("[get] topic    = %s\n", v3.value_or("(null)").c_str());

            // Overwrite a key
            store.put("name", "Bob");
            auto v4 = store.get("name");
            printf("[get] name (after overwrite) = %s\n", v4.value_or("(null)").c_str());

            // Delete a key
            store.remove("level");
            auto v5 = store.get("level");
            printf("[get] level (after remove) = %s (expected null)\n", v5.value_or("(null)").c_str());

            // List keys
            auto k = store.keys();
            printf("[keys] Active keys (%zu): ", k.size());
            for (size_t i = 0; i < k.size(); ++i) {
                printf("%s%s", k[i].c_str(), (i + 1 < k.size()) ? ", " : "");
            }
            printf("\n\n");
        }
        // store destructor called -> msync + munmap + close

        // Phase 2: Reopen and verify persistence
        {
            printf("[reopen] Verifying persistence after close/reopen...\n");
            SimpleKVStore store2(db_path);

            auto v1 = store2.get("name");
            auto v2 = store2.get("language");
            auto v3 = store2.get("topic");
            auto v4 = store2.get("level");

            printf("[get] name     = %s (expected Bob)\n", v1.value_or("(null)").c_str());
            printf("[get] language = %s (expected C++)\n", v2.value_or("(null)").c_str());
            printf("[get] topic    = %s (expected mmap)\n", v3.value_or("(null)").c_str());
            printf("[get] level    = %s (expected null - was deleted)\n", v4.value_or("(null)").c_str());

            auto k = store2.keys();
            printf("[keys] Active keys (%zu): ", k.size());
            for (size_t i = 0; i < k.size(); ++i) {
                printf("%s%s", k[i].c_str(), (i + 1 < k.size()) ? ", " : "");
            }
            printf("\n");
        }

        // Cleanup
        ::unlink(db_path);
        printf("\n");
    }

    static void run_all() {
        printf("############################################################\n");
        printf("# Stage 9: Comprehensive Practical Projects                #\n");
        printf("############################################################\n\n");
        demo_mini_grep();
        demo_kv_store();
        printf("############################################################\n");
        printf("# Stage 9: All demos complete                              #\n");
        printf("############################################################\n");
    }
};

} // namespace mmap_learn
