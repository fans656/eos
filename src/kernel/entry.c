#include "types.h"
#include "stdio.h"

#define KERNEL_BASE 0xc0000000

#define PAGE_SIZE 4096

#define PTE_P 0x001
#define PTE_W 0x002
#define PTE_PS 0x080

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
            :: "c"((uint)page_dir - KERNEL_BASE));
    clear_console();
    main();
    asm("hlt");
}
