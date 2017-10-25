#include "string.h"

void* memset(void* ptr, uchar value, uint cnt) {
    if (!(cnt & 3) && !((uint)ptr & 3)) {
        asm volatile(
                "mov eax, %0;"
                "mov edi, %1;"
                "mov ecx, %2;"
                "cld;"
                "rep stosd;"
                :: "a"((uint)value), "b"(ptr), "c"(cnt >> 2));
        return ptr;
    } else if (!(cnt & 1) && !((uint)ptr & 1)) {
        asm volatile(
                "mov ax, %0;"
                "mov edi, %1;"
                "mov ecx, %2;"
                "cld;"
                "rep stosw;"
                :: "a"((ushort)value), "b"(ptr), "c"(cnt >> 1));
        return ptr;
    } else {
        asm volatile(
                "mov al, %0;"
                "mov edi, %1;"
                "mov ecx, %2;"
                "cld;"
                "rep stosb;"
                :: "a"(value), "b"(ptr), "c"(cnt));
        return ptr;
    }
}

void* memmove(void* dst, const void* src, uint cnt) {
    if (!(cnt & 3) && !((uint)dst & 3) && !((uint)src & 3)) {
        return memcpy_dword(dst, src, cnt);
    } else if (!(cnt & 1) && !((uint)dst & 1) && !((uint)src & 1)) {
        return memcpy_word(dst, src, cnt);
    } else {
        return memcpy_byte(dst, src, cnt);
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
