#include "stdio.h"

int printf(char* fmt, ...) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_PRINTF));
}

FILE* fopen(const char* path, const char* mode) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_FOPEN));
}

int fclose(FILE* fp) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_FCLOSE));
}

size_t fread(void* buffer, size_t size, size_t count, FILE* fp) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_FREAD));
}

size_t fwrite(const void* data, size_t size, size_t count, FILE* fp) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_FWRITE));
}

size_t fsize(FILE* fp) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_FSIZE));
}

void* load_file(const char* path) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_LOAD_FILE));
}
