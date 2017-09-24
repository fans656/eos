#include "conf.h"

int printf(char* fmt, ...) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_PRINTF));
}
