#include <stdint.h>
#include "malloc.h"
#include "io.h"
#include "util.h"
#include "constants.h"

uint32_t cur = HEAP_BEG;

uint32_t* page_directory = (uint32_t*)(1 * MB);
uint32_t* page_table = (uint32_t*)(1 * MB + 4 * KB);

void enable_paging() {
    for (int i = 0; i < 1024; ++i) {
        uint32_t address = ((uint32_t)(page_table) + i * 4 * KB) | 3;
        page_directory[i] = address;
    }
    for (int i_table = 0; i_table < 1024; ++i_table) {
        for (int i_entry = 0; i_entry < 1024; ++i_entry) {
            uint32_t address = i_table * 4 * MB + i_entry * 4 * KB;
            *(uint32_t*)((uint32_t)page_table + i_table * 4 * KB + i_entry * 4) = address | 3;
        }
    }
    *(uint32_t*)((uint32_t)page_table + 4 * KB * 256) = 3;
    asm volatile(
            "mov eax, %0;"
            "mov cr3, eax;"
            "mov eax, cr0;"
            "or eax, 0x80000001;"
            "mov cr0, eax;"
            :: "g"((uint32_t)page_directory)
            );
}

void init_memory() {
}

void* malloc(uint32_t size) {
    uint32_t res = cur;
    cur += size;
    return (void*)res;
}

void free(void* p) {
}
