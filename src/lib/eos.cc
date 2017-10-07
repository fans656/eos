#include "eos.h"

void* _get_message(int id, bool blocking) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_GET_MESSAGE));
}

void* get_message(int id, bool blocking) {
    if (!blocking) {
        return _get_message(id, blocking);
    } else {
        while (true) {
            auto msg = _get_message(id, blocking);
            if (msg) {
                return msg;
            }
        }
    }
}

void put_message(int id, void* message) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_PUT_MESSAGE));
}

void memory_blit(
        const char* buffer, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_MEMORY_BLIT));
}
