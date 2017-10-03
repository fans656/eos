#include "stdlib.h"

void* malloc(size_t size) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_MALLOC));
}

void free(void* addr) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_FREE));
}

void exit(int status) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_EXIT));
}

extern int main();

void _start() {
    exit(main());
}
