#include "string.h"
#include "stdio.h"

void* memset(void* ptr, uchar value, uint cnt) {
    uchar* p = (uchar*)ptr;
    while (cnt--) {
        *p++ = value;
    }
    return ptr;
}

void* memmove(void* dst, const void* src, uint cnt) {
    if (!(cnt & 3) && !((uint)dst & 3) && !((uint)src & 3)) {
        asm volatile(
                "mov esi, %0;"
                "mov edi, %1;"
                "mov ecx, %2;"
                "cld;"
                "rep movsd;"
                :: "a"(src), "b"(dst), "c"(cnt >> 2));
        return dst;
    } else if (!(cnt & 1) && !((uint)dst & 1) && !((uint)src & 1)) {
        asm volatile(
                "mov esi, %0;"
                "mov edi, %1;"
                "mov ecx, %2;"
                "cld;"
                "rep movsw;"
                :: "a"(src), "b"(dst), "c"(cnt >> 1));
        return dst;
    } else {
        asm volatile(
                "mov esi, %0;"
                "mov edi, %1;"
                "mov ecx, %2;"
                "cld;"
                "rep movsb;"
                :: "a"(src), "b"(dst), "c"(cnt));
        return dst;
    }
}

void* strcpy(char* dst, const char* src) {
    while (*src) {
        *dst++ = *src++;
    }
    *dst = 0;
    return dst;
}

void* strncpy(char* dst, const char* src, size_t n) {
    while (n--) {
        *dst++ = *src++;
    }
    *dst = 0;
    return dst;
}

size_t strlen(const char* s) {
    size_t n = 0;
    while (*s++) {
        ++n;
    }
    return n;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        ++s1;
        ++s2;
    }
    return *s1 - *s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n-- && *s1 && *s2) {
        if (*s1 != *s2) {
            return *s1 - *s2;
        }
        ++s1;
        ++s2;
    }
    return 0;
}
