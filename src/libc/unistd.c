#include "unistd.h"

void sleep(int ms) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_SLEEP));
}
