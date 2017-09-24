#include "string.h"

void* memset(void* ptr, uchar value, uint cnt) {
    uchar* p = (uchar*)ptr;
    while (cnt--) {
        *p++ = value;
    }
}
