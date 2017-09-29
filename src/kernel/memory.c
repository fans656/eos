#include "memory.h"
#include "conf.h"
#include "stdio.h"
#include "string.h"
#include "util.h"

#define ROUND_DOWN(x) ((uint)((x) - (uint)(x) % PAGE_SIZE))
#define ROUND_UP(x) (((uint)(x) + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE)

#define align4(x) (((x) + 3) / 4 * 4)

uint* kernel_end = (uint*)(ENTRY_INFO_ADDR + KERNEL_BASE);

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
            *pde = V2P(pt) | PTE_P | PTE_W;
        } else {  // page table already exist
            pt = (uint*)P2V(*pde & 0xfffff000);
        }
        uint* page = alloc_frame();
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

typedef struct Header {
    uint size;
    uchar* addr;
    struct Header* prev;
    struct Header* next;
    uchar data[0];
} Header;

#define HEADER_SIZE sizeof(Header)

// heap is at [PAGE_SIZE, PHY_MEM)
// leave first page unmmaped to allow 0 represent invalid pointer
uchar* end = (uchar*)PAGE_SIZE;

void* sbrk(int incr) {
    if (!incr) {
        return end;
    }
    uchar* old_end = end;
    end += incr;
    if (incr > 0) {
        if (ROUND_DOWN(old_end) != ROUND_UP(end)) {
            map_pages(pgdir, ROUND_UP(old_end), ROUND_UP(end) - ROUND_UP(old_end));
        }
    } else {
        if (ROUND_UP(end) != ROUND_UP(old_end)) {
            unmap_pages(pgdir, ROUND_UP(end), ROUND_UP(old_end) - ROUND_UP(end));
        }
    }
    reload_cr3(pgdir);
    return old_end;
}

Header _head, _tail;
Header* head = &_head;
Header* tail = &_tail;

void* malloc(size_t size) {
    size = align4(size);
    Header* p = head->next;
    while (p->size && p->size < size + HEADER_SIZE) {
        p = p->next;
    }
    // if no free block, ask kernel for more space
    if (!p->size) {
        p = (Header*)sbrk(size + HEADER_SIZE);
        p->size = size + HEADER_SIZE;
        p->addr = p->data;
        return p->addr;
    }
    // if block is large enough, split it
    if (p->size - size - HEADER_SIZE >= HEADER_SIZE + 4) {
        Header* q = (Header*)((uchar*)p + p->size);
        q->size = p->size - size - HEADER_SIZE;
        q->addr = q->data;
        q->prev = p;
        q->next = p->next;
        p->size = size + HEADER_SIZE;
        p->next = q;
    }
    // take the block
    p->prev->next = p->next;
    p->next->prev = p->prev;
    return p->addr;
}

// return first if merged, else second
Header* try_merge(Header* p, Header* q) {
    if ((uint)p + p->size == (uint)q) {
        p->size += q->size;
        p->next = q->next;
        p->next->prev = p;
        return p;
    }
    return q;
}

void free(void* addr) {
    Header* cur = (Header*)((uchar*)addr - HEADER_SIZE);
    Header* nex = head->next;
    while (nex->size && nex->addr < cur->addr) {
        nex = nex->next;
    }
    cur->prev = nex->prev;
    cur->next = nex;
    cur->prev->next = cur;
    cur->next->prev = cur;
    try_merge(cur, cur->next);  // for "a b c", merge "b c" first, else it will be wrong
    cur = try_merge(cur->prev, cur);
    // never give back
    //// if at heap end, give it back to kernel
    //if (!cur->next->size) {
    //    cur->prev->next = cur->next;
    //    cur->next->prev = cur->prev;
    //    sbrk(-cur->size);
    //}
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

void init_memory() {
    // use higher half GDT
    asm("lgdt [%0]" :: "r"(&gdtdesc));
    // free the identity mapping 0~4MB
    pgdir[0] = 0;
    // map all physical space so kernel code can manipulate them
    for (int i = 0; i < PHY_MEM / 4; ++i) {
        pgdir[i + (KERNEL_BASE >> 22)] = i * 4 * MB | PTE_P | PTE_W | PTE_PS;
    }
    reload_cr3(pgdir);
    // free frames list
    for (uint vaddr = P2V(PHY_MEM * MB) - PAGE_SIZE; vaddr >= *kernel_end; vaddr -= PAGE_SIZE) {
        free_frame(vaddr);
    }
    // init malloc free list
    head->addr = 0;
    tail->addr = (uchar*)0xffffffff;
    head->size = tail->size = 0;
    head->prev = tail->next = 0;
    head->next = tail;
    tail->prev = head;
}
