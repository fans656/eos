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

void* operator new(size_t n) {
    return malloc(n);
}

void operator delete(void* p) {
    free(p);
}

void* operator new[](size_t n) {
    return malloc(n);
}

void operator delete[](void* p) {
    free(p);
}

extern int main();

extern "C" {
void _start() {
    exit(main());
}
}
