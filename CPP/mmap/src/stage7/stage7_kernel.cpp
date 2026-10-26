#include "stage7_kernel.h"

#include "../log.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifdef __APPLE__
#include <unistd.h>
#endif

namespace mmap_learn {

namespace {
constexpr const char* kTag = "Stage7";
} // namespace

// ============================================================
// ConsoleObserver
// ============================================================

const char *ConsoleObserver::event_type_str(MemoryEventType t) {
    switch (t) {
    case MemoryEventType::MAPPED:    return "MAPPED";
    case MemoryEventType::UNMAPPED:  return "UNMAPPED";
    case MemoryEventType::PROTECTED: return "PROTECTED";
    case MemoryEventType::SYNCED:    return "SYNCED";
    case MemoryEventType::LOCKED:    return "LOCKED";
    }
    return "UNKNOWN";
}

void ConsoleObserver::on_event(const MemoryEvent &event) {
    LOGD(kTag, "[EVENT]");
    LOGI(kTag, "        %-10s | addr: %p | size: %8zu | %s", event_type_str(event.event_type), event.address,
         event.size, event.description.c_str());
}

// ============================================================
// ObservableMmapRegion
// ============================================================

void ObservableMmapRegion::notify(MemoryEventType type, const std::string &desc) {
    MemoryEvent event{type, region_.data(), region_.size(), desc};
    for (auto *obs : observers_) {
        obs->on_event(event);
    }
}

void ObservableMmapRegion::add_observer(IMemoryObserver *obs) { observers_.push_back(obs); }

void ObservableMmapRegion::remove_observer(IMemoryObserver *obs) {
    observers_.erase(std::remove(observers_.begin(), observers_.end(), obs), observers_.end());
}

void ObservableMmapRegion::map(size_t length, int prot, int flags, int fd, off_t offset) {
    region_ = MmapRegion(nullptr, length, prot, flags, fd, offset);
    notify(MemoryEventType::MAPPED, "mmap created");
}

void ObservableMmapRegion::unmap() {
    notify(MemoryEventType::UNMAPPED, "munmap requested");
    region_ = MmapRegion(); // 移动赋值：旧映射在 operator= 中被 munmap
}

int ObservableMmapRegion::protect(int prot) {
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

int ObservableMmapRegion::sync(int flags) {
    int rc = region_.sync(flags);
    std::string desc = (flags == MS_SYNC) ? "msync MS_SYNC" : "msync MS_ASYNC";
    desc += (rc == 0) ? " (ok)" : " (failed)";
    notify(MemoryEventType::SYNCED, desc);
    return rc;
}

int ObservableMmapRegion::lock() {
    int rc = region_.lock();
    std::string desc = std::string("mlock") + ((rc == 0) ? " (ok)" : " (failed)");
    notify(MemoryEventType::LOCKED, desc);
    return rc;
}

void *ObservableMmapRegion::data() { return region_.data(); }

const void *ObservableMmapRegion::data() const { return region_.data(); }

size_t ObservableMmapRegion::size() const { return region_.size(); }

bool ObservableMmapRegion::valid() const { return region_.valid(); }

// ============================================================
// ProcessMapsReader
// ============================================================

std::vector<ProcessMapEntry> ProcessMapsReader::read_maps() {
    std::vector<ProcessMapEntry> entries;

#ifdef __linux__
    // unique_ptr 管理 FILE*：parse_maps_file 中途抛异常（如内存不足）也会 fclose
    std::unique_ptr<FILE, int (*)(FILE *)> f(fopen("/proc/self/maps", "r"), &fclose);
    if (!f) {
        LOGE(kTag, "[ProcessMapsReader] Cannot open /proc/self/maps");
        return entries;
    }
    parse_maps_file(f.get(), entries);
#elif defined(__APPLE__)
    // macOS 上没有 /proc/self/maps。
    // 仍然尝试打开（个别环境会模拟它），打不开就提示使用 vmmap 命令。
    std::unique_ptr<FILE, int (*)(FILE *)> f(fopen("/proc/self/maps", "r"), &fclose);
    if (f) {
        parse_maps_file(f.get(), entries);
    } else {
        LOGW(kTag, "    [ProcessMapsReader] /proc/self/maps not available on macOS.");
        LOGI(kTag, "    [ProcessMapsReader] Run manually:  vmmap %d", static_cast<int>(getpid()));
    }
#else
    LOGW(kTag, "    [ProcessMapsReader] Unsupported platform");
#endif
    return entries;
}

void ProcessMapsReader::print_maps() {
    auto entries = read_maps();
    if (entries.empty()) {
        LOGW(kTag, "    (no entries available)");
        return;
    }
    LOGD(kTag, "%-34s %-5s %-10s %-8s %-10s %s", "Address Range", "Perms", "Offset", "Dev", "Inode", "Pathname");
    LOGD(kTag, "%s", std::string(100, '-').c_str());
    for (const auto &e : entries) {
        LOGD(kTag, "%012lx-%012lx %-5s %08lx   %-8s %-10lu %s", e.addr_start, e.addr_end, e.perms, e.offset,
             e.device, e.inode, e.pathname.c_str());
    }
}

const ProcessMapEntry *ProcessMapsReader::find_region(const std::vector<ProcessMapEntry> &entries, void *addr) {
    auto target = reinterpret_cast<unsigned long>(addr);
    for (const auto &e : entries) {
        if (target >= e.addr_start && target < e.addr_end) {
            return &e;
        }
    }
    return nullptr;
}

void ProcessMapsReader::parse_maps_file(FILE *f, std::vector<ProcessMapEntry> &entries) {
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        ProcessMapEntry entry{};
        char path_buf[256] = {};
        // 每行格式：起始地址-结束地址 权限 偏移 设备号 inode 路径
        int n = sscanf(line, "%lx-%lx %4s %lx %11s %lu %255[^\n]", &entry.addr_start, &entry.addr_end,
                       entry.perms, &entry.offset, entry.device, &entry.inode, path_buf);
        if (n >= 6) {
            if (n == 7) {
                // 去掉路径前面的空白（maps 用空格对齐路径列）
                char *p = path_buf;
                while (*p == ' ' || *p == '\t') ++p;
                entry.pathname = p;
            }
            entries.push_back(std::move(entry));
        }
    }
}

// ============================================================
// Stage7Kernel
// ============================================================

void Stage7Kernel::demo_observer() {
    LOGD(kTag, "1:demo_observer: Observer pattern for mmap operations");

    ConsoleObserver console_obs;
    ObservableMmapRegion region;
    region.add_observer(&console_obs);

    // 1. 映射一块匿名内存：内核只新建 VMA，还没有物理页
    LOGI(kTag, "        --- Mapping anonymous region (4096 bytes) ---");
    region.map(4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON);

    // 2. 写数据：首次写入触发缺页，内核分配一页清零的物理内存（do_anonymous_page）
    LOGI(kTag, "        --- Writing data ---");
    memset(region.data(), 0xAB, 64);
    LOGI(kTag, "        (wrote 64 bytes of 0xAB, no event -- direct access)");

    // 3. 改为只读：此后写入会触发 SIGSEGV
    LOGI(kTag, "        --- Changing protection to READ-ONLY ---");
    region.protect(PROT_READ);

    // 4. 改回读写
    LOGI(kTag, "        --- Changing protection to READ-WRITE ---");
    region.protect(PROT_READ | PROT_WRITE);

    // 5. 同步：匿名映射没有对应文件，msync 无实际作用（多数系统直接返回成功），这里只为演示事件
    LOGI(kTag, "        --- Syncing ---");
    region.sync(MS_SYNC);

    // 6. 锁定页，防止被换出
    LOGI(kTag, "        --- Locking pages ---");
    region.lock();

    // 7. 解除映射：删除 VMA，释放物理页
    LOGI(kTag, "    --- Unmapping ---");
    region.unmap();
}

void Stage7Kernel::demo_process_maps() {
    LOGD(kTag, "     ");
    LOGD(kTag, "     ");
    LOGD(kTag, "2:demo_process_maps: Process memory layout");

    // 创建两块映射，稍后在 maps 中查找它们
    MmapRegion region1(nullptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    MmapRegion region2(nullptr, 8192, PROT_READ, MAP_PRIVATE | MAP_ANON, -1, 0);

    LOGI(kTag, "    Created region1 at %p (4096 bytes, rw)", region1.data());
    LOGI(kTag, "    Created region2 at %p (8192 bytes, r-)", region2.data());

    // 打印进程的全部映射
    LOGD(kTag, "--- Current process memory mappings ---");
    ProcessMapsReader::print_maps();

    // 按地址查找我们的两块映射落在哪个 VMA 里
    auto entries = ProcessMapsReader::read_maps();
    if (!entries.empty()) {
        LOGD(kTag, "--- Looking up our regions ---");
        const ProcessMapEntry *e1 = ProcessMapsReader::find_region(entries, region1.data());
        if (e1) {
            LOGI(kTag, "    region1 (%p) found in: %012lx-%012lx %s %s", region1.data(), e1->addr_start, e1->addr_end,
                 e1->perms, e1->pathname.c_str());
        } else {
            LOGW(kTag, "    region1 (%p) not found in maps (expected on macOS)", region1.data());
        }

        const ProcessMapEntry *e2 = ProcessMapsReader::find_region(entries, region2.data());
        if (e2) {
            LOGI(kTag, "    region2 (%p) found in: %012lx-%012lx %s %s", region2.data(), e2->addr_start, e2->addr_end,
                 e2->perms, e2->pathname.c_str());
        } else {
            LOGW(kTag, "    region2 (%p) not found in maps (expected on macOS)", region2.data());
        }
    }
}

void Stage7Kernel::demo_vma_concepts() {
    LOGD(kTag, "     ");
    LOGD(kTag, "     ");
    LOGD(kTag, "3:demo_vma_concepts: Kernel internals explanation 内核原理讲解");

    LOGI(kTag, "=== vm_area_struct (VMA) - Linux Kernel Data Structure:Linux 内核里描述一段映射的数据结构  ===");
    LOGI(kTag, "    Each call to mmap() creates a vm_area_struct in the kernel.每调用一次 mmap()，内核就新建一个 vm_area_struct。");
    LOGI(kTag, "    Key fields:主要字段:");
    LOGI(kTag, "      vm_start / vm_end   - virtual address range:虚拟地址范围（起始、结束）");
    LOGI(kTag, "      vm_flags            - permissions (VM_READ, VM_WRITE, VM_EXEC, VM_SHARED):权限和属性（可读、可写、可执行、是否共享）");
    LOGI(kTag, "      vm_pgoff            - offset into the file (in pages):在文件中的偏移，以页为单位");
    LOGI(kTag, "      vm_file             - pointer to struct file (NULL for anonymous):指向被映射文件的 struct file，匿名映射为 NULL");
    LOGI(kTag, "      vm_ops              - operations table (fault, map_pages, etc.:回调函数表，例如缺页时调用的 fault、map_pages)");
    LOGI(kTag, "    VMAs are stored in a maple tree (>= 6.1) or rb-tree (older kernels)");
    LOGI(kTag, "        VMA 存放在 maple tree（内核 6.1 及以后）或红黑树（更早的内核）中，");
    LOGI(kTag, "    within mm_struct, allowing O(log n) lookup by address.");
    LOGI(kTag, "        挂在进程的 mm_struct 上，按地址查找是 O(log n)。");

    LOGD(kTag, "=== do_mmap() - Kernel Path When You Call mmap() 调用 mmap() 后内核走的路径===");
    LOGI(kTag, "    User calls mmap()  -->  syscall entry (SYS_mmap)");
    LOGI(kTag, "        用户态调用 mmap()  -->  进入系统调用入口（SYS_mmap）");
    LOGI(kTag, "      --> ksys_mmap_pgoff()");
    LOGI(kTag, "      -->                    :根据 fd 取出 struct file，处理 MAP_HUGETLB");
    LOGI(kTag, "        --> vm_mmap_pgoff()");
    LOGI(kTag, "        -->                 :加 mmap 写锁，做安全检查");
    LOGI(kTag, "          --> do_mmap()");
    LOGI(kTag, "          -->           ：真正建立映射的函数");
    LOGI(kTag, "            1. Validate parameters (length, flags, prot):检查参数（长度、标志、权限）");
    LOGI(kTag, "            2. Find free virtual address range (get_unmapped_area):找一段空闲的虚拟地址（get_unmapped_area）");
    LOGI(kTag, "            3. Check against rlimits (RLIMIT_AS, RLIMIT_MEMLOCK):检查资源限制（地址空间上限、可锁定内存上限）");
    LOGI(kTag, "            4. Allocate vm_area_struct:分配一个 vm_area_struct");
    LOGI(kTag, "            5. Insert VMA into mm_struct's maple tree:把 VMA 插入 mm_struct 的 maple tree");
    LOGI(kTag, "            6. If file-backed: call file->f_op->mmap():如果映射的是文件：调用该文件系统或驱动的 f_op->mmap()");
    LOGI(kTag, "            7. Return virtual address to userspace:把虚拟地址返回给用户态");
    LOGV(kTag, "            NOTE: No physical pages are allocated yet! (demand paging)注意：到这里还没有分配任何物理页！（按需分页）");

    LOGV(kTag, "    === Page Fault Handling - When Memory Is Actually Allocated 缺页处理：物理内存真正被分配的时刻===");
    LOGI(kTag, "    First access to mapped page triggers a page fault:第一次访问映射里的某一页时，触发缺页异常：");
    LOGI(kTag, "      CPU exception  -->  do_page_fault()");
    LOGI(kTag, "                        CPU 产生异常  -->  进入 do_page_fault()");
    LOGI(kTag, "        --> find_vma(mm, address)");
    LOGI(kTag, "        -->                     ：找出出错地址属于哪个 VMA，找不到就发 SIGSEGV");
    LOGI(kTag, "          --> handle_mm_fault()");
    LOGI(kTag, "          -->                   ：通用的缺页处理入口");
    LOGI(kTag, "            --> __handle_mm_fault()");
    LOGI(kTag, "            -->                 ：逐级分配页表（PGD/PUD/PMD）");
    LOGI(kTag, "              --> handle_pte_fault()");
    LOGI(kTag, "              -->               ：根据最后一级页表项的状态分情况处理");
    LOGI(kTag, "    Depending on mapping type: 按映射类型分成几种情况");
    LOGI(kTag, "      1.Anonymous   :  do_anonymous_page() - allocate zeroed page");
    LOGI(kTag, "                       匿名映射：            - 分配一页清零的物理内存");
    LOGI(kTag, "      2.File-backed : do_read_fault()      - read page from disk");
    LOGI(kTag, "                      文件映射：             - 从 page cache 取页，不在就从磁盘读入");
    LOGI(kTag, "      3.COW         :  do_wp_page()        - copy page on write");
    LOGI(kTag, "                       写时复制：            - 写->只读的共享页时复制出一份私有页");
    LOGI(kTag, "      4.Swap        :  do_swap_page()      - read from swap space");
    LOGI(kTag, "                       换出的页：            - 从 swap 分区读回内存");
    LOGV(kTag, "    After handling, the PTE (Page Table Entry) is updated,处理完后更新页表项（PTE），");
    LOGV(kTag, "    and the CPU retries the faulting instruction.CPU 回到用户态，重新执行刚才出错的那条指令。");


    LOGI(kTag, "    === Summary 总结===");
    LOGI(kTag, "     mmap() is lazy -- it only sets up bookkeeping.mmap() 是\"懒\"的：它只在内核里登记一下。");
    LOGI(kTag, "     Physical memory is allocated on-demand via page faults.物理内存要等访问时通过缺页按需分配。");
    LOGI(kTag, "     This is why mmap() returns instantly even for huge mappings.所以哪怕映射一个很大的文件，mmap() 也能立刻返回。");
}

void Stage7Kernel::run_all() {
    LOGI(kTag, "========================================");
    LOGI(kTag, "Stage 7: Kernel Internals Observation");
    LOGI(kTag, "========================================");

    demo_observer();
    demo_process_maps();
    demo_vma_concepts();

    LOGI(kTag, "========================================");
    LOGI(kTag, "Stage 7 complete");
    LOGI(kTag, "========================================");
}

} // namespace mmap_learn
