#include "eos.h"

void* get_message(int id, bool blocking) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_GET_MESSAGE));
}
