#include "time.h"
#include "def.h"

void timeit(const char* fmt, ...) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_TIMEIT));
}
