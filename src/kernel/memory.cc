#include "memory.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "util.h"
#include "math.h"
#include "process.h"
#include "time.h"

#if 0
#define STATIC_MALLOC
#endif

#define ROUND_DOWN(x) ((uint)((x) - (uint)(x) % PAGE_SIZE))
#define ROUND_UP(x) (((uint)(x) + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE)

uint* p_kernel_end = (uint*)(0x500 + KERNEL_BASE);
uint kernel_end;

///////////////////////////////////////////////////////// frames

typedef struct Frame {
    struct Frame* next;
} Frame;

Frame* free_frame_list = 0;

void* alloc_frame() {
    void* res = free_frame_list;
    if (!free_frame_list) {
        panic("=PANIC= alloc_frame: no more frames!");
    }
    free_frame_list = free_frame_list->next;
    return res;
}

void free_frame(uint vaddr) {
    Frame* frame = (Frame*)vaddr;
    frame->next = free_frame_list;
    free_frame_list = frame;
}

void reload_cr3(void* pgdir) {
    asm volatile("mov cr3, %0" :: "r"(V2P(pgdir)));
}

///////////////////////////////////////////////////////// pages

// `vbeg` and `size` are of byte granularity
void map_pages(uint pgdir[], uint vbeg, size_t size) {
    uint pbeg = ROUND_DOWN(vbeg) / PAGE_SIZE;
    uint pend = ROUND_UP(vbeg + size) / PAGE_SIZE;
    for (uint p = pbeg; p != pend; ++p) {
        uint* pde = &pgdir[p >> 10];
        uint* pt;
        if (!(*pde & PTE_P)) {  // page table not present
            pt = (uint*)alloc_frame();
            memset(pt, 0, PAGE_SIZE);
            *pde = V2P(pt) | PTE_P | PTE_W;
        } else {  // page table present
            pt = (uint*)P2V(*pde & 0xfffff000);
        }
        uint* page = (uint*)alloc_frame();
        memset(page, 0, PAGE_SIZE);
        pt[p & 0x3ff] = V2P(page) | PTE_P | PTE_W;
    }
}

// `vbeg` and `size` are of byte granularity
void unmap_pages(uint pgdir[], uint vbeg, uint size) {
    uint pbeg = ROUND_DOWN(vbeg) / PAGE_SIZE;
    uint pend = ROUND_UP(vbeg + size) / PAGE_SIZE;
    for (uint p = pbeg; p != pend; ++p) {
        uint* pde = &pgdir[p >> 10];
        if (!(*pde & PTE_P)) {
            p += 1023;
            continue;
        }
        uint* pt = (uint*)P2V(*pde & 0xfffff000);
        uint* pte = &pt[p & 0x3ff];
        if (!(*pte & PTE_P)) {
            continue;
        }
        uint frame = *pte & 0xfffff000;
        free_frame(P2V(frame));
        *pte = 0;
    }
}

////////////////////////////////////////////////////// malloc

#define HEAP_SIZE (128 * MB)

// leave first page unmmaped to allow 0 represent invalid pointer
uint sbrk_end = KERNEL_HEAP_VADDR;
uint sbrk_page_end = KERNEL_HEAP_VADDR;

void* sbrk(int incr) {
    uint* pgdir = running_proc ? running_proc->pgdir : kernel_pgdir;
    uint end = sbrk_end + incr;
    if (end < KERNEL_HEAP_VADDR || end > KERNEL_HEAP_VADDR + HEAP_SIZE) {
        panic("==PANIC== sbrk() to %x", end);
    }
    if (!incr) {
        return (void*)sbrk_end;
    }
    uint old_end = sbrk_end;
    if (incr > 0) {
        uint left = sbrk_end;
        uint right = left + incr;
        uint page_right = ROUND_UP(right);
        if (page_right > sbrk_page_end) {
            map_pages(pgdir, sbrk_page_end, page_right - sbrk_page_end);
            sbrk_page_end = page_right;
            reload_cr3(pgdir);
        }
        sbrk_end = right;
    } else {
        uint right = sbrk_end;
        uint left = right + incr;
        uint page_left = ROUND_UP(left);
        if (sbrk_page_end > page_left) {
            unmap_pages(pgdir, page_left, sbrk_page_end - page_left);
            sbrk_page_end = page_left;
            reload_cr3(pgdir);
        }
        sbrk_end = left;
    }
    return (void*)old_end;
}

typedef struct Header {
    uint used;
    uint size;  // the block size with header excluded, i.e. malloc(size) but 4-bytes aligned
    uchar* addr;
    const char* name;
    struct Header* prev;
    struct Header* next;
    uchar data[0];
} Header;
#define HEADER_SIZE sizeof(Header)

Header _head, _tail;
Header* head = &_head;
Header* tail = &_tail;

void insert_after(Header* p, Header* q) {
    q->prev = p;
    q->next = p->next;
    q->prev->next = q->next->prev = q;
}

void remove(Header* p) {
    p->prev->next = p->next;
    p->next->prev = p->prev;
}

void try_merge(Header* p, Header* q) {
    if (!p->used && !q->used && (uchar*)p + p->size + HEADER_SIZE == (uchar*)q) {
        p->size += q->size;
        p->next = q->next;
        p->next->prev = p;
    }
}

#ifdef STATIC_MALLOC

uchar* static_malloc_base = (uchar*)P2V(64 * MB);
size_t static_malloc_size = 0;

void* named_malloc(size_t size, const char* name) {
    size = align4(size);
    void* res = static_malloc_base + static_malloc_size;
    static_malloc_size += size;
    return res;
}

void free(void* addr) {
    return;
}

#else

void* named_malloc(size_t size, const char* name) {
    size = align4(size);
    Header* p = head->next;
    while (p->size && (p->used || p->size < size)) {
        p = p->next;
    }
    // if no free block, ask kernel for more space
    if (!p->size) {
        p = (Header*)sbrk(size + HEADER_SIZE);
        p->used = false;
        p->size = size;
        p->addr = p->data;
        insert_after(tail->prev, p);
    }
    // if block is large enough, split it
    if (p->size - size >= HEADER_SIZE + 4) {
        Header* q = (Header*)((uchar*)p + HEADER_SIZE + size);
        q->used = false;
        q->size = p->size - HEADER_SIZE - size;
        q->addr = q->data;
        q->name = p->name;
        insert_after(p, q);
        p->size -= HEADER_SIZE + q->size;
    }
    p->used = true;
    p->name = name;
    return p->addr;
}

void free(void* addr) {
    Header* cur = (Header*)((uchar*)addr - HEADER_SIZE);
    cur->used = false;

    // the order is important
    // if we `try_merge(cur->prev, cur)` first
    // then `cur` may not point to the right node then
    try_merge(cur, cur->next);
    try_merge(cur->prev, cur);

    Header* last = tail->prev;
    if (!last->used) {
        remove(last);
        sbrk(-last->size);
    }
}

#endif

void* malloc(size_t size) {
    return named_malloc(size, "N/A");
}

///////////////////////////////////////////////////////// new/delete

// https://stackoverflow.com/questions/8186018/how-to-properly-replace-global-new-delete-operators

void* operator new(size_t n) {
    return malloc(n);
}

void operator delete(void* p) {
    free(p);
}

void operator delete(void* p, uint _) {
    free(p);
}

void* operator new[](size_t n) {
    return malloc(n);
}

void operator delete[](void* p) {
    free(p);
}

void operator delete[](void* p, uint _) {
    free(p);
}

//void _Unwind_Resume(

///////////////////////////////////////////////////////// GDT

// http://wiki.osdev.org/Global_Descriptor_Table
#define Descriptor(base, limit, attr) \
    (struct GDTEntry) { \
        (limit) & 0xffff, \
        (base) & 0xffff, \
        ((base) >> 16) & 0xff, \
        (attr) & 0xff, \
        (((attr) >> 8) & 0xf0) | ((limit) >> 16) & 0x0f, \
        (base) >> 24 \
    }

struct __attribute__((__packed__)) GDTEntry {
    ushort limit;
    ushort base_low;
    uchar base_mid;
    uchar access;
    uchar flags_limit_high;
    uchar base_high;
} gdt[] = {
    Descriptor(0,0,0),
    Descriptor(0,0xffffffff,0x409a),  // code
    Descriptor(0,0xffffffff,0x4092),  // data
};

struct __attribute__((__packed__)) GDTDesc {
    ushort size;
    uint offset;
} gdtdesc = {sizeof(gdt) - 1, (uint)gdt};

///////////////////////////////////////////////////////// init

// http://www.uruk.org/orig-grub/mem64mb.html
typedef struct MemRange {
    uint addr_low;
    uint addr_high;
    uint len_low;
    uint len_high;
    uint type;  // 1 available, 2 reserved
} MemRange;

#define MEM_RANGE_AVAILBLE 1
#define MEM_RANGE_RESERVED 2

void init_memory() {
    kernel_end = *p_kernel_end;
    // use higher half GDT
    asm("lgdt [%0]" :: "r"(&gdtdesc));
    // free the identity mapping 0~4MB
    kernel_pgdir[0] = 0;
    // determine usable memory
    uint phy_mem_size = 0;
    MemRange* usable_mem = 0;
    uint count = *(uint*)P2V(0x7004);
    MemRange* range = (MemRange*)P2V(0x7008);
    while (count--) {
        //assert(range->addr_high == 0);
        //assert(range->len_high == 0);
        uint beg = range->addr_low;
        uint end = range->addr_low + range->len_low;
        if (usable_mem == 0 && end - beg > 64 * MB) {
            usable_mem = range;
        }
        phy_mem_size = max(phy_mem_size, end);
        ++range;
    }
    // map all physical space so kernel code can manipulate them
    for (uint paddr = 0; paddr < phy_mem_size; paddr += 4 * MB) {
        kernel_pgdir[(KERNEL_BASE + paddr) >> 22] = paddr | PTE_P | PTE_W | PTE_PS;
    }
    // free frames list
    uint free_beg = max(kernel_end, usable_mem->addr_low);
    uint free_end = usable_mem->addr_low + usable_mem->len_low;
    for (uint vaddr = P2V(free_end) - PAGE_SIZE; vaddr >= free_beg; vaddr -= PAGE_SIZE) {
        free_frame(vaddr);
    }
    // alloc page table to heap pdes so every process's kernel pgdir will be in sync
    uint* pde = &kernel_pgdir[KERNEL_HEAP_VADDR >> 22];
    for (uint paddr = 0; paddr < phy_mem_size; paddr += 4 * MB) {
        uint* page = (uint*)alloc_frame();
        memset(page, 0, PAGE_SIZE);
        *pde++ = V2P(page) | PTE_P | PTE_W;
    }
    // init malloc free list
    head->addr = 0;
    tail->addr = (uchar*)0xffffffff;
    head->used = tail->used = true;
    head->name = "head";
    tail->name = "tail";
    head->size = tail->size = 0;
    head->prev = tail->next = 0;
    head->next = tail;
    tail->prev = head;

    reload_cr3(kernel_pgdir);
}

void test_32_mb_write_speed_test() {
    asm("sti");
    size_t size = 32 * MB;
    auto p = new uchar[size];
    while (true) {
        timeit(0);
        for (int i = 0; i < size; ++i) {
            p[i] = i;
        }
        timeit("32MB write");
    }
}
