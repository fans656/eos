#include "stdio.h"
#include "constants.h"

int printf(char* fmt, ...) {
    asm volatile ("mov ebx, %0; mov eax, %1" :: "g"(&fmt), "g"(SYSCALL_PRINTF));
    asm("int 0x80");
}
