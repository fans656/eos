#include "time.h"
#include "def.h"

uint clock() {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_CLOCK));
}

void timeit(const char* fmt, ...) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_TIMEIT));
}
