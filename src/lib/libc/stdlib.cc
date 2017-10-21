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

void operator delete(void* p, uint _) {
    free(p);
}

void* operator new[](size_t n) {
    return malloc(n);
}

void operator delete[](void* p) {
    free(p);
}

extern int main();

void _start() {
    exit(main());
}

int cur_rand_val;

int rand() {
    cur_rand_val = (22695477 * (uint)cur_rand_val + 1) % RAND_MAX;
    return cur_rand_val;
}

void srand(uint seed) {
    cur_rand_val = seed;
    rand();
}

int randint(int min, int max) {
    uint range = max - min + 1;
    if (range <= RAND_MAX) {
       return min + rand() % range;
    } else {
        int n = range / RAND_MAX;
        int i = randint(0, n);
        return RAND_MAX * i + randint(0, (i == n) ? (range % RAND_MAX) : (RAND_MAX - 1));
    }
}
