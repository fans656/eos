#include "string.h"

void* memset(void* ptr, uchar value, uint cnt) {
    uchar* p = (uchar*)ptr;
    while (cnt--) {
        *p++ = value;
    }
    return ptr;
}

void* memcpy(void* dst, void* src, uint cnt) {
    uchar* p = src;
    uchar* q = dst;
    while (cnt--) {
        *q++ = *p++;
    }
    return dst;
}
