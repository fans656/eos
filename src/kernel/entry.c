#include "types.h"
#include "stdio.h"
#include "conf.h"

__attribute__((__aligned__(PAGE_SIZE))) uint pgdir[1024] = {
    [0] = 0 | PTE_P | PTE_W | PTE_PS,
    [KERNEL_BASE >> 22] = 0 | PTE_P | PTE_W | PTE_PS
};

uint* kernel_end = (uint*)0xc0000500;

void main();

void entry() {
    asm volatile(
            // enable 4M page
            "mov eax, cr4;"
            "or eax, 0x10;"
            "mov cr4, eax;"
            // load page directory
            "mov eax, %0;"
            "mov cr3, eax;"
            // enable paging
            "mov eax, cr0;"
            "or eax, 0x80010000;"
            "mov cr0, eax;"
            // setup stack
            "mov eax, [0x500];"
            "mov esp, eax;"
            // jump to main
            "mov eax, %1;"
            "jmp eax"
            :: "c"((uint)pgdir - KERNEL_BASE), "i"(main));
}
