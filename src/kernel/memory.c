#include "memory.h"
#include "conf.h"
#include "stdio.h"
#include "string.h"
#include "util.h"

extern uint* kernel_end;

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

typedef struct Frame {
    struct Frame* next;
} Frame;

Frame* free_frame_list = 0;

void free_frame(uint vaddr) {
    Frame* frame = (Frame*)vaddr;
    frame->next = free_frame_list;
    free_frame_list = frame;
}

void* alloc_frame() {
    void* res = free_frame_list;
    if (free_frame_list) {
        free_frame_list = free_frame_list->next;
    }
    return res;
}

void reload_cr3(void* pgdir) {
    asm volatile("mov cr3, %0" :: "r"(V2P(pgdir)));
}

void init_memory() {
    for (uint vaddr = *kernel_end; vaddr < P2V(4 * MB); vaddr += PAGE_SIZE) {
        free_frame(vaddr);
    }
    pgdir[0] = 0;
    reload_cr3(pgdir);
    asm("lgdt [%0]" :: "r"(&gdtdesc));
}

void map_pages(uint pgdir[], uint vbeg, uint size) {
    uint pbeg = vbeg / PAGE_SIZE;
    uint pend = (vbeg + size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint p = pbeg; p != pend; ++p) {
        uint* pde = &pgdir[p >> 10];
        uint* pt;
        if (*pde == 0) {
            pt = alloc_frame();
            *pde = V2P(pt) | PTE_P | PTE_W;
        } else {
            pt = (uint*)P2V(*pde & 0xfffff000);
        }
        pt[p & 0x3ff] = V2P(alloc_frame()) | PTE_P | PTE_W;
    }
}
