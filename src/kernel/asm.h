#include "types.h"

static inline uchar inb(ushort port) {
    uchar res;
    asm volatile("inb %0, %1" : "=a"(res) : "d"(port));
    return res;
}

static inline void outb(ushort port, uchar val) {
    asm volatile("outb %0, %1" :: "d"(port), "a"(val));
}

static inline ushort inw(ushort port) {
    ushort res;
    asm volatile ("inw %0, %1" : "=a"(res) : "d"(port));
    return res;
}

static inline void outw(ushort port, ushort val) {
    asm volatile ("outw %0, %1" :: "dN"(port), "a"(val));
}
