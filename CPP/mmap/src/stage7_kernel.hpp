#pragma once

#include "mmap_wrapper.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <unistd.h>
#endif

namespace mmap_learn {

// ============================================================
// MemoryEvent: describes a single mmap-related operation
// ============================================================
enum class MemoryEventType { MAPPED, UNMAPPED, PROTECTED, SYNCED, LOCKED };

struct MemoryEvent {
    MemoryEventType event_type;
    void *address;
    size_t size;
    std::string description;
};

// ============================================================
// IMemoryObserver: observer interface
// ============================================================
class IMemoryObserver {
public:
    virtual ~IMemoryObserver() = default;
    virtual void on_event(const MemoryEvent &event) = 0;
};

// ============================================================
// ConsoleObserver: prints events to stdout with formatted output
// ============================================================
class ConsoleObserver : public IMemoryObserver {
    static const char *event_type_str(MemoryEventType t) {
        switch (t) {
        case MemoryEventType::MAPPED:    return "MAPPED";
        case MemoryEventType::UNMAPPED:  return "UNMAPPED";
        case MemoryEventType::PROTECTED: return "PROTECTED";
        case MemoryEventType::SYNCED:    return "SYNCED";
        case MemoryEventType::LOCKED:    return "LOCKED";
        }
        return "UNKNOWN";
    }

public:
    void on_event(const MemoryEvent &event) override {
        printf("  [EVENT] %-10s | addr: %p | size: %8zu | %s\n",
               event_type_str(event.event_type), event.address, event.size, event.description.c_str());
    }
};

// ============================================================
// ObservableMmapRegion: wraps MmapRegion, notifies observers
// ============================================================
class ObservableMmapRegion {
    MmapRegion region_;
    std::vector<IMemoryObserver *> observers_;

    void notify(MemoryEventType type, const std::string &desc) {
        MemoryEvent event{type, region_.data(), region_.size(), desc};
        for (auto *obs : observers_) {
            obs->on_event(event);
        }
    }

public:
    ObservableMmapRegion() = default;

    void add_observer(IMemoryObserver *obs) { observers_.push_back(obs); }

    void remove_observer(IMemoryObserver *obs) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), obs), observers_.end());
    }

    // Map: build a region via MmapBuilder parameters, then notify
    void map(size_t length, int prot, int flags, int fd = -1, off_t offset = 0) {
        region_ = MmapRegion(nullptr, length, prot, flags, fd, offset);
        notify(MemoryEventType::MAPPED, "mmap created");
    }

    // Unmap: destroy the current region
    void unmap() {
        notify(MemoryEventType::UNMAPPED, "munmap requested");
        region_ = MmapRegion(); // destructor of old region calls munmap
    }

    // Protect: change memory protection
    int protect(int prot) {
        int rc = region_.protect(prot);
        std::string desc = "mprotect prot=";
        if (prot & PROT_READ)  desc += "R";
        if (prot & PROT_WRITE) desc += "W";
        if (prot & PROT_EXEC)  desc += "X";
        if (prot == PROT_NONE) desc += "NONE";
        desc += (rc == 0) ? " (ok)" : " (failed)";
        notify(MemoryEventType::PROTECTED, desc);
        return rc;
    }

    // Sync: flush to disk
    int sync(int flags = MS_SYNC) {
        int rc = region_.sync(flags);
        std::string desc = (flags == MS_SYNC) ? "msync MS_SYNC" : "msync MS_ASYNC";
        desc += (rc == 0) ? " (ok)" : " (failed)";
        notify(MemoryEventType::SYNCED, desc);
        return rc;
    }

    // Lock: lock pages in memory
    int lock() {
        int rc = region_.lock();
        std::string desc = std::string("mlock") + ((rc == 0) ? " (ok)" : " (failed)");
        notify(MemoryEventType::LOCKED, desc);
        return rc;
    }

    // Accessors
    void *data() { return region_.data(); }
    const void *data() const { return region_.data(); }
    size_t size() const { return region_.size(); }
    bool valid() const { return region_.valid(); }

    template <typename T>
    T *as() {
        return region_.as<T>();
    }
};

// ============================================================
// ProcessMapEntry: one line from /proc/self/maps
// ============================================================
struct ProcessMapEntry {
    unsigned long addr_start;
    unsigned long addr_end;
    char perms[5];     // e.g. "rwxp"
    unsigned long offset;
    char device[12];   // e.g. "08:01"
    unsigned long inode;
    std::string pathname;
};

// ============================================================
// ProcessMapsReader: reads and parses process memory mappings
// ============================================================
class ProcessMapsReader {
public:
    static std::vector<ProcessMapEntry> read_maps() {
        std::vector<ProcessMapEntry> entries;

#ifdef __linux__
        FILE *f = fopen("/proc/self/maps", "r");
        if (!f) {
            printf("  [ProcessMapsReader] Cannot open /proc/self/maps\n");
            return entries;
        }
        parse_maps_file(f, entries);
        fclose(f);
#elif defined(__APPLE__)
        // On macOS, /proc/self/maps does not exist.
        // Try the file anyway (some environments emulate it), otherwise advise vmmap.
        FILE *f = fopen("/proc/self/maps", "r");
        if (f) {
            parse_maps_file(f, entries);
            fclose(f);
        } else {
            printf("  [ProcessMapsReader] /proc/self/maps not available on macOS.\n");
            printf("  [ProcessMapsReader] Run manually:  vmmap %d\n", static_cast<int>(getpid()));
        }
#else
        printf("  [ProcessMapsReader] Unsupported platform\n");
#endif
        return entries;
    }

    static void print_maps() {
        auto entries = read_maps();
        if (entries.empty()) {
            printf("  (no entries available)\n");
            return;
        }
        printf("  %-34s %-5s %-10s %-8s %-10s %s\n", "Address Range", "Perms", "Offset", "Dev", "Inode",
               "Pathname");
        printf("  %s\n", std::string(100, '-').c_str());
        for (const auto &e : entries) {
            printf("  %012lx-%012lx %-5s %08lx   %-8s %-10lu %s\n", e.addr_start, e.addr_end, e.perms, e.offset,
                   e.device, e.inode, e.pathname.c_str());
        }
    }

    static const ProcessMapEntry *find_region(const std::vector<ProcessMapEntry> &entries, void *addr) {
        auto target = reinterpret_cast<unsigned long>(addr);
        for (const auto &e : entries) {
            if (target >= e.addr_start && target < e.addr_end) {
                return &e;
            }
        }
        return nullptr;
    }

private:
    static void parse_maps_file(FILE *f, std::vector<ProcessMapEntry> &entries) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            ProcessMapEntry entry{};
            char path_buf[256] = {};
            // Format: addr_start-addr_end perms offset dev inode pathname
            int n = sscanf(line, "%lx-%lx %4s %lx %11s %lu %255[^\n]", &entry.addr_start, &entry.addr_end,
                           entry.perms, &entry.offset, entry.device, &entry.inode, path_buf);
            if (n >= 6) {
                if (n == 7) {
                    // Trim leading whitespace from pathname
                    char *p = path_buf;
                    while (*p == ' ' || *p == '\t') ++p;
                    entry.pathname = p;
                }
                entries.push_back(std::move(entry));
            }
        }
    }
};

// ============================================================
// Stage7Kernel: demo methods for kernel internals observation
// ============================================================
class Stage7Kernel {
public:
    // ============================================================
    // Demo 1: Observer pattern on mmap operations
    // ============================================================
    static void demo_observer() {
        printf("[Stage7] demo_observer: Observer pattern for mmap operations\n\n");

        ConsoleObserver console_obs;
        ObservableMmapRegion region;
        region.add_observer(&console_obs);

        // 1. Map an anonymous region
        printf("  --- Mapping anonymous region (4096 bytes) ---\n");
        region.map(4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON);

        // 2. Write some data
        printf("  --- Writing data ---\n");
        memset(region.data(), 0xAB, 64);
        printf("  (wrote 64 bytes of 0xAB, no event -- direct access)\n");

        // 3. Change protection to read-only
        printf("  --- Changing protection to READ-ONLY ---\n");
        region.protect(PROT_READ);

        // 4. Change back to read-write
        printf("  --- Changing protection to READ-WRITE ---\n");
        region.protect(PROT_READ | PROT_WRITE);

        // 5. Sync (will fail on anonymous, but demonstrates the event)
        printf("  --- Syncing ---\n");
        region.sync(MS_SYNC);

        // 6. Lock pages
        printf("  --- Locking pages ---\n");
        region.lock();

        // 7. Unmap
        printf("  --- Unmapping ---\n");
        region.unmap();

        printf("\n");
    }

    // ============================================================
    // Demo 2: Read process memory mappings
    // ============================================================
    static void demo_process_maps() {
        printf("[Stage7] demo_process_maps: Process memory layout\n\n");

        // Create a couple of mappings so we can find them
        MmapRegion region1(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        MmapRegion region2(nullptr, 8192, PROT_READ, MAP_PRIVATE | MAP_ANON, -1, 0);

        printf("  Created region1 at %p (4096 bytes, rw)\n", region1.data());
        printf("  Created region2 at %p (8192 bytes, r-)\n", region2.data());
        printf("\n");

        // Print all mappings
        printf("  --- Current process memory mappings ---\n");
        ProcessMapsReader::print_maps();
        printf("\n");

        // Find which mapping contains our pointers
        auto entries = ProcessMapsReader::read_maps();
        if (!entries.empty()) {
            printf("  --- Looking up our regions ---\n");
            const ProcessMapEntry *e1 = ProcessMapsReader::find_region(entries, region1.data());
            if (e1) {
                printf("  region1 (%p) found in: %012lx-%012lx %s %s\n", region1.data(), e1->addr_start,
                       e1->addr_end, e1->perms, e1->pathname.c_str());
            } else {
                printf("  region1 (%p) not found in maps (expected on macOS)\n", region1.data());
            }

            const ProcessMapEntry *e2 = ProcessMapsReader::find_region(entries, region2.data());
            if (e2) {
                printf("  region2 (%p) found in: %012lx-%012lx %s %s\n", region2.data(), e2->addr_start,
                       e2->addr_end, e2->perms, e2->pathname.c_str());
            } else {
                printf("  region2 (%p) not found in maps (expected on macOS)\n", region2.data());
            }
        }

        printf("\n");
    }

    // ============================================================
    // Demo 3: Explain VMA concepts and kernel mmap path
    // ============================================================
    static void demo_vma_concepts() {
        printf("[Stage7] demo_vma_concepts: Kernel internals explanation\n\n");

        printf("  ================================================================\n");
        printf("  vm_area_struct (VMA) - Linux Kernel Data Structure\n");
        printf("  ================================================================\n");
        printf("  Each call to mmap() creates a vm_area_struct in the kernel.\n");
        printf("  Key fields:\n");
        printf("    vm_start / vm_end   - virtual address range\n");
        printf("    vm_flags            - permissions (VM_READ, VM_WRITE, VM_EXEC, VM_SHARED)\n");
        printf("    vm_pgoff            - offset into the file (in pages)\n");
        printf("    vm_file             - pointer to struct file (NULL for anonymous)\n");
        printf("    vm_ops              - operations table (fault, map_pages, etc.)\n");
        printf("  VMAs are stored in a maple tree (>= 6.1) or rb-tree (older kernels)\n");
        printf("  within mm_struct, allowing O(log n) lookup by address.\n");
        printf("\n");

        printf("  ================================================================\n");
        printf("  do_mmap() - Kernel Path When You Call mmap()\n");
        printf("  ================================================================\n");
        printf("  User calls mmap()  -->  syscall entry (SYS_mmap)\n");
        printf("    --> ksys_mmap_pgoff()\n");
        printf("      --> vm_mmap_pgoff()\n");
        printf("        --> do_mmap()\n");
        printf("          1. Validate parameters (length, flags, prot)\n");
        printf("          2. Find free virtual address range (get_unmapped_area)\n");
        printf("          3. Check against rlimits (RLIMIT_AS, RLIMIT_MEMLOCK)\n");
        printf("          4. Allocate vm_area_struct\n");
        printf("          5. Insert VMA into mm_struct's maple tree\n");
        printf("          6. If file-backed: call file->f_op->mmap()\n");
        printf("          7. Return virtual address to userspace\n");
        printf("  NOTE: No physical pages are allocated yet! (demand paging)\n");
        printf("\n");

        printf("  ================================================================\n");
        printf("  Page Fault Handling - When Memory Is Actually Allocated\n");
        printf("  ================================================================\n");
        printf("  First access to mapped page triggers a page fault:\n");
        printf("    CPU exception  -->  do_page_fault()\n");
        printf("      --> find_vma(mm, address)\n");
        printf("        --> handle_mm_fault()\n");
        printf("          --> __handle_mm_fault()\n");
        printf("            --> handle_pte_fault()\n");
        printf("  Depending on mapping type:\n");
        printf("    Anonymous:   do_anonymous_page() - allocate zeroed page\n");
        printf("    File-backed: do_read_fault()     - read page from disk\n");
        printf("    COW:         do_wp_page()        - copy page on write\n");
        printf("    Swap:        do_swap_page()      - read from swap space\n");
        printf("  After handling, the PTE (Page Table Entry) is updated,\n");
        printf("  and the CPU retries the faulting instruction.\n");
        printf("\n");

        printf("  ================================================================\n");
        printf("  Summary: mmap() is lazy -- it only sets up bookkeeping.\n");
        printf("  Physical memory is allocated on-demand via page faults.\n");
        printf("  This is why mmap() returns instantly even for huge mappings.\n");
        printf("  ================================================================\n");

        printf("\n");
    }

    // ============================================================
    // Run all Stage 7 demos
    // ============================================================
    static void run_all() {
        printf("========================================\n");
        printf("  Stage 7: Kernel Internals Observation\n");
        printf("========================================\n\n");

        demo_observer();
        demo_process_maps();
        demo_vma_concepts();

        printf("========================================\n");
        printf("  Stage 7 complete\n");
        printf("========================================\n");
    }
};

} // namespace mmap_learn
