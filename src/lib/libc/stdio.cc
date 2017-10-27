#include "stdio.h"

int printf(const char* fmt, ...) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_PRINTF));
}

int debug(const char* fmt, ...) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_DEBUG));
}

void panic(const char* fmt, ...) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_PRINTF));
    while (true) {
        asm("hlt");
    }
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

void hexdump(const void* addr, uint cnt) {
    const uchar* p = (const uchar*)addr;
    for (int i = 0; i < cnt; ++i) {
        if (i % 16) {
            printf(" ");
            if (i % 8 == 0) {
                printf(" ");
            }
        }
        if (i && i % 16 == 0) {
            printf("\n");
        }
        printf("%1x", *p++);
    }
    printf("\n");
}
