#include "types.h"
#include "stdio.h"
#include "conf.h"

__attribute__((__aligned__(PAGE_SIZE))) uint page_dir[1024] = {
    [0] = 0 | PTE_P | PTE_W | PTE_PS,
    [KERNEL_BASE >> 22] = 0 | PTE_P | PTE_W | PTE_PS
};

uint* kernel_end = (uint*)0xc0000500;

void main();

void entry() {
    asm volatile(
            // enable 4M page
            "mov %%cr4, %%eax;"
            "orl $0x10, %%eax;"
            "mov %%eax, %%cr4;"
            // load page directory
            "mov %0, %%eax;"
            "mov %%eax, %%cr3;"
            // enable paging
            "mov %%cr0, %%eax;"
            "orl $0x80010000, %%eax;"
            "mov %%eax, %%cr0;"
            // setup stack
            "mov 0x500, %%eax;"
            "mov %%eax, %%esp;"
            "movl %1, %%eax; jmp *%%eax"
            :: "c"((uint)page_dir - KERNEL_BASE), "i"(main));
}
