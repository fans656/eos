#include "types.h"
#include "conf.h"
#include "stdio.h"
#include "string.h"
#include "util.h"

extern uint* kernel_end;
extern uint page_dir[];

typedef struct Frame {
    struct Frame* next;
} Frame;

Frame* free_frame_list = 0;

void free_frame(void* vaddr) {
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

void reload_cr3(void* page_dir) {
    asm volatile("movl %0, %%cr3" :: "r"(V2P_UINT(page_dir)));
}

void init_vm() {
    for (uchar* vaddr = (uchar*)*kernel_end;
            vaddr < P2V(4 * MB); vaddr += PAGE_SIZE) {
        free_frame(vaddr);
    }
    page_dir[0] = 0;
    reload_cr3(page_dir);
}
