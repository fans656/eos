#include "memory.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "util.h"
#include "assert.h"
#include "math.h"

#define ROUND_DOWN(x) ((uint)((x) - (uint)(x) % PAGE_SIZE))
#define ROUND_UP(x) (((uint)(x) + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE)

#define align4(x) (((x) + 3) / 4 * 4)

uint* kernel_end = (uint*)(0x500 + KERNEL_BASE);

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
        if (!(*pde & PTE_P)) {  // page table absent
            pt = alloc_frame();
            memset(pt, 0, PAGE_SIZE);
            *pde = V2P(pt) | PTE_P | PTE_W;
        } else {  // page table already exist
            pt = (uint*)P2V(*pde & 0xfffff000);
        }
        uint* page = alloc_frame();
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
            panic("unmap non-mapped page!");
        }
        uint* pt = (uint*)P2V(*pde & 0xfffff000);
        uint* pte = &pt[p & 0x3ff];
        if (!(*pte & PTE_P)) {
            panic("unmap non-mapped page!");
        }
        uint frame = *pte & 0xfffff000;
        free_frame(P2V(frame));
        *pte = 0;
    }
}

////////////////////////////////////////////////////// malloc

#define HEAP_SIZE (128 * MB)

// leave first page unmmaped to allow 0 represent invalid pointer
uint sbrk_end = PAGE_SIZE;
uint sbrk_page_end = PAGE_SIZE;

void* sbrk(int incr) {
    uint end = sbrk_end + incr;
    if (end < PAGE_SIZE || end > HEAP_SIZE) {
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
        }
        sbrk_end = right;
    } else {
        uint right = sbrk_end;
        uint left = right + incr;
        uint page_left = ROUND_UP(left);
        if (sbrk_page_end > page_left) {
            unmap_pages(pgdir, page_left, sbrk_page_end - page_left);
            sbrk_page_end = page_left;
        }
        sbrk_end = left;
    }
    reload_cr3(pgdir);
    return (void*)old_end;
}

typedef struct Header {
    uint used;
    uint size;
    uchar* addr;
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

void delete(Header* p) {
    p->prev->next = p->next;
    p->next->prev = p->prev;
}

void* malloc(size_t size) {
    size = align4(size);
    Header* p = head->next;
    while (p->size && (p->used || p->size < size + HEADER_SIZE)) {
        p = p->next;
    }
    // if no free block, ask kernel for more space
    if (!p->size) {
        p = (Header*)sbrk(size + HEADER_SIZE);
        p->used = false;
        p->size = size + HEADER_SIZE;
        p->addr = p->data;
        insert_after(tail->prev, p);
    }
    // if block is large enough, split it
    if (p->size - size - HEADER_SIZE >= HEADER_SIZE + 4) {
        Header* q = (Header*)((uchar*)p + size + HEADER_SIZE);
        q->used = false;
        q->size = p->size - size - HEADER_SIZE;
        q->addr = q->data;
        insert_after(p, q);
        p->size -= q->size;
    }
    p->used = true;
    return p->addr;
}

void try_merge(Header* p, Header* q) {
    if (!p->used && !q->used && (uchar*)p + p->size == (uchar*)q) {
        p->size += q->size;
        p->next = q->next;
        p->next->prev = p;
    }
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
        delete(last);
        sbrk(-last->size);
    }
}

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

struct __attribute__((__packed__)) {
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
    // use higher half GDT
    asm("lgdt [%0]" :: "r"(&gdtdesc));
    // free the identity mapping 0~4MB
    pgdir[0] = 0;
    // determine usable memory
    uint phy_mem_size = 0;
    MemRange* usable_mem = 0;
    uint count = *(uint*)P2V(0x7004);
    MemRange* range = (MemRange*)P2V(0x7008);
    while (count--) {
        assert(range->addr_high == 0);
        assert(range->len_high == 0);
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
        pgdir[(KERNEL_BASE + paddr) >> 22] = paddr | PTE_P | PTE_W | PTE_PS;
    }
    reload_cr3(pgdir);
    // free frames list
    uint free_beg = max(*kernel_end, usable_mem->addr_low);
    uint free_end = usable_mem->addr_low + usable_mem->len_low;
    for (uint vaddr = P2V(free_end) - PAGE_SIZE; vaddr >= free_beg; vaddr -= PAGE_SIZE) {
        free_frame(vaddr);
    }
    // init malloc free list
    head->addr = 0;
    tail->addr = (uchar*)0xffffffff;
    head->used = tail->used = true;
    head->size = tail->size = 0;
    head->prev = tail->next = 0;
    head->next = tail;
    tail->prev = head;
}

////////////////////////////////////////////////////////////////////// test

void dump_page(uint* p) {
    for (int i = 0; i < 1024; ++i) {
        if (i && i % 64 == 0) putchar('\n');
        if (i && i % 256 == 0) putchar('\n');
        if (i && i % 64 != 0 && i % 16 == 0) putchar(' ');
        printf("%d", *p ? 1 : 0);
        *p++;
    }
    putchar('\n');
}

void dump_pagedir(uint* pgdir) {
    dump_page(pgdir);
}

void dump_pagetable(uint* pgtbl, size_t i) {
    if (!(pgdir[i] & PTE_P)) {
        panic("Page table at pgdir[%d] does not exist\n", i);
    }
    dump_page((uint*)P2V(pgdir[i] & 0xfffff000));
}

void assert_mapped(uint* pgdir, uint i_pde, uint i_pte_beg, uint i_pte_end) {
    assert(pgdir[i_pde] & PTE_P);

    uint* pt = (uint*)P2V(pgdir[i_pde] & 0xfffff000);
    for (int i = i_pte_beg; i != i_pte_end; ++i) {
        assert(pt[i] & PTE_P);
    }
    for (int i = 0; i < i_pte_beg; ++i ) {
        assert(!(pt[i] & PTE_P));
    }
    for (int i = i_pte_end; i < 1024; ++i ) {
        assert(!(pt[i] & PTE_P));
    }
}

void test_map_unmap(uint beg, uint size) {
    printf(".. test_map_unmap %x %x", beg, size);

    map_pages(pgdir, beg, size);

    uint i_pde_beg = beg / (4 * MB);
    uint i_pde_end = (beg + size + 4 * MB - 1) / (4 * MB);
    
    uint i_page = ROUND_DOWN(beg) / (4 * KB);
    uint i_page_end = ROUND_UP(beg + size) / (4 * KB);
    
    uint wnd_beg = i_page / 1024 * 1024;
    uint wnd_end = wnd_beg + 1024;

    for (int i_pde = i_pde_beg; i_pde != i_pde_end; ++i_pde) {
        uint pte_beg = max(wnd_beg, i_page) % 1024;
        uint pte_end = min(wnd_end, i_page_end) % 1024;
        pte_end += pte_end ? 0 : 1024;
        assert_mapped(pgdir, i_pde, pte_beg, pte_end);
        i_page += pte_end - pte_beg;
        wnd_beg += 1024;
        wnd_end += 1024;
    }
    for (int i = 0; i < 1024 - 256; ++i) {
        if (i < i_pde_beg && i >= i_pde_end) {
            assert(!(pgdir[i] & PTE_P));
        }
    }

    unmap_pages(pgdir, beg, size);
    for (int i_pde = i_pde_beg; i_pde != i_pde_end; ++i_pde) {
        assert_mapped(pgdir, i_pde, 0, 0);
        pgdir[i_pde] = 0;
    }

    printf("\rOK\n");
}

void do_test_map_unmap() {
    uint begs[] = {
        0, 1,
        4 * KB - 1, 4 * KB, 4 * KB + 1,
        4 * MB - 1, 4 * MB, 4 * MB + 1,
        32 * MB - 1, 32 * MB, 32 * MB + 1
    };
    for (int i = 0; i < sizeof(begs) / sizeof(begs[0]); ++i) {
        uint beg = begs[i];
        test_map_unmap(beg, 1);
        test_map_unmap(beg, 2);
        test_map_unmap(beg, 4 * KB);
        test_map_unmap(beg, 4 * KB + 1);
        test_map_unmap(beg, 4 * MB);
        test_map_unmap(beg, 4 * MB + 1);
        test_map_unmap(beg, 16 * MB);
        test_map_unmap(beg, 16 * MB + 1);
    }
    printf("do_test_map_unmap finished!\n");
}

void dump_malloc_list() {
    printf("============================== dump_malloc_list\n");
    for (Header* p = head; p; p = p->next) {
        printf("%x addr: %x, size: %d, used: %d, next: %x\n", p, p->addr, p->size, p->used, p->next);
    }
    printf("============================== end dump_malloc_list\n");
}

void do_test_malloc_free() {
    const int MAX_SIZE = 60 * MB;
    const int MAX_ALLOC = 4 * MB + 1;
    const int N = 1024;
    uchar* a[N];
    int n = 0;
    uint size = 0;
    for (int _ = 0; _ < 1000; ++_) {
        bool allocating = false;
        if (n == 0) {
            allocating = true;
        } else if (size + MAX_ALLOC + HEADER_SIZE > MAX_SIZE) {
            allocating = false;
        } else {
            allocating = randint(0, 9) < randint(0, 9);
        }
        if (allocating) {
            uint alloc_size = randint(1, MAX_ALLOC);
            uchar* p = malloc(alloc_size);
            size += ((Header*)(p - HEADER_SIZE))->size;
            a[n] = p;
            ++n;
        } else {
            int i = randint(0, n - 1);
            size -= ((Header*)(a[i] - HEADER_SIZE))->size;
            free(a[i]);
            a[i] = a[--n];
        }
    }
}

void memory_test() {
    //do_test_map_unmap();
    //do_test_malloc_free();
    printf("memory_test finished!\n");
}
