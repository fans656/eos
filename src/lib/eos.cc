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
