#include "eos.h"
#include "string.h"

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

uint set_timer(uint ms, uint id, bool singleshot) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_SET_TIMER));
}

void memory_blit(
        const uchar* buffer, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_BLIT));
}

void get_screen_info(ScreenInfo* info) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_GET_SCREEN_INFO));
}

void execute_(char* fpath) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_EXECUTE));
}

void execute(const char* fpath) {
    char* pfpath = new char[256];
    strcpy(pfpath, fpath);
    execute_(pfpath);
    delete pfpath;
}
