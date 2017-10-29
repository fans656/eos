#ifndef ASM_H
#define ASM_H

#include "def.h"

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

static inline uint ind(ushort port) {
    uint res;
    asm volatile ("ind %0, %1" : "=a"(res) : "d"(port));
    return res;
}

static inline void outd(ushort port, uint val) {
    asm volatile ("outd %0, %1" :: "dN"(port), "a"(val));
}

static inline void wait_for(int port, uchar mask, uchar val) {
    while ((inb(port) & mask) != val) {
        ;
    }
}

static inline void insd(int port, void* addr, int cnt) {
    asm volatile("cld; rep insd"
            : "=D"(addr), "=c"(cnt)
            : "d"(port), "0"(addr), "1"(cnt)
            : "memory", "cc");
}

static inline void stosb(void* addr, uint count, uchar val) {
    asm volatile("cld; rep stosb"
            : "=D"(addr), "=c"(count)
            : "0"(addr), "1"(count), "a"(val)
            : "memory", "cc");
}

#endif
