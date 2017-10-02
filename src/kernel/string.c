#include "string.h"

void* memset(void* ptr, uchar value, uint cnt) {
    uchar* p = (uchar*)ptr;
    while (cnt--) {
        *p++ = value;
    }
    return ptr;
}

void* memcpy(void* dst, const void* src, uint cnt) {
    const uchar* p = src;
    uchar* q = dst;
    while (cnt--) {
        *q++ = *p++;
    }
    return dst;
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
