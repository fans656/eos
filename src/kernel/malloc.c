#include <stdint.h>
#include "malloc.h"
#include "io.h"
#include "util.h"
#include "constants.h"

typedef struct {
    uint32_t* frames_top;
} MemoryMeta;

typedef struct {
    uint32_t tables_p[1024];
    uint32_t tables_v[1024];
    uint32_t paddr;
} PageDirectory;

MemoryMeta* memory_meta = (MemoryMeta*)MEMORY_META_ADDR;

PageDirectory* page_directory = (PageDirectory*)PAGE_DIRECTORY_ADDR;

uint32_t* frames_top;

static uint32_t heap_end = HEAP_BEG;
static uint32_t mapped_heap_end = HEAP_BEG;

void reload_cr3() {
    asm volatile("mov eax, %0; mov cr3, eax" :: "m"(page_directory->paddr));
}

void show_memory_map() {
    PageDirectory* page_directory = (PageDirectory*)PAGE_DIRECTORY_ADDR;
    uint32_t vaddr = 0;
    uint32_t paddr_beg = 0, paddr_end = 0;
    bool found = false;
    for (int i = 0; i < 1024 * 1024;) {
        int i_pde = (vaddr & 0xffc00000) >> 22;
        int i_pte = (vaddr & 0x003ff000) >> 12;
        if ((page_directory->tables_p)[i_pde]) {
            uint32_t* page_table = (uint32_t*)(page_directory->tables_v)[i_pde];
            uint32_t pte = page_table[i_pte] & 0xfffff000;
            if (!found) {
                if (page_table[i_pte]) {
                    paddr_beg = pte;
                    paddr_end = paddr_beg + 4 * KB;
                    found = true;
                }
            } else {
                if (pte == paddr_end) {
                    paddr_end = pte + 4 * KB;
                } else {
                    if (found) {
                        printf("%x-%x: %x-%x\n",
                                vaddr - (paddr_end - paddr_beg), vaddr,
                                paddr_beg, paddr_end);
                    }
                    found = false;
                }
            }
            vaddr += 4 * KB;
            ++i;
        } else {
            if (found) {
                printf("%x-%x: %x-%x\n",
                        vaddr - (paddr_end - paddr_beg), vaddr,
                        paddr_beg, paddr_end);
            }
            found = false;
            vaddr += 4 * MB;
            i += 1024;
        }
    }
    if (found) {
        printf("%x-%x: %x-%x\n",
                vaddr - (paddr_end - paddr_beg), vaddr,
                paddr_beg, paddr_end);
    }
}

void unmap_pages(uint32_t vaddr, uint32_t size) {
    return;
    PageDirectory* page_directory = (PageDirectory*)PAGE_DIRECTORY_ADDR;
    while (size >= 4 * MB) {
        int i_pde = (vaddr & 0xffc00000) >> 22;
        (page_directory->tables_p)[i_pde] = 0;
        size -= 4 * MB;
        vaddr += 4 * MB;
    }
    int i_pde = (vaddr & 0xffc00000) >> 22;
    int i_pte = (vaddr & 0x003ff000) >> 12;
    uint32_t* pte = (uint32_t*)((page_directory->tables_v)[i_pde] + i_pte * 4);
    while (size) {
        *pte++ = 0;
        size -= 4 * KB;
    }
    reload_cr3();
}

void init_memory() {
    *(uint32_t*)PAGE_DIRECTORY_ADDR = 0;
    asm("invlpg [0]");
    asm("mov word ptr [0xc00b8000], 0x0f41");
    hlt();
    
    //uint32_t eip;
    //asm volatile("mov eax, [ebp + 4]; mov %0, eax" : "=m"(eip));
    //panic("%x\n", eip);

    printf("hi\n");
}

uint32_t alloc_frame() {
}

void* malloc(uint32_t size) {
    if (heap_end >= mapped_heap_end) {
        uint32_t frame_paddr = alloc_frame();
    }
}

void free(void* p) {
}

void* syscall_sbrk(int shift) {
}
