#include "def.h"

void* memset(void* ptr, uchar value, uint cnt);
void* memcpy(void* dst, const void* src, uint cnt);

void* strcpy(char* dst, const char* src);
void* strncpy(char* dst, const char* src, size_t n);
size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

static inline void* memcpy_dword(void* dst, const void* src, uint cnt) {
    asm volatile(
            "mov esi, %0;"
            "mov edi, %1;"
            "mov ecx, %2;"
            "cld;"
            "rep movsd;"
            :: "a"(src), "b"(dst), "c"(cnt >> 2)
            );
    return dst;
}
