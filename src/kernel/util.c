#include <stdint.h>
#include "util.h"

uint32_t g_current_random_value;

void hlt() {
    while (1) {
        asm("hlt");
    }
}

uint8_t inb(uint16_t port) {
    uint8_t res;
    asm volatile ("inb %0, %1" : "=a"(res) : "d"(port));
    return res;
}

void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" :: "dN"(port), "a"(val));
}

uint16_t inw(uint16_t port) {
    uint16_t res;
    asm volatile ("inw %0, %1" : "=a"(res) : "d"(port));
    return res;
}

void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" :: "dN"(port), "a"(val));
}

void srand(uint32_t seed) {
    g_current_random_value = seed;
    rand();
}

uint32_t rand() {
    g_current_random_value = (22695477 * g_current_random_value + 1) % RAND_MAX;
    return g_current_random_value;
}

int randint(int min, int max) {
    return min + rand() % (max - min + 1);
}

int strcmp(char* src, char* dst) {
    while (*src && *dst) {
        if (*src < *dst) {
            return -1;
        } else if (*src > *dst) {
            return 1;
        }
        ++src;
        ++dst;
    }
    return *src - *dst;
}

int strncmp(char* src, char* dst, int n) {
    while (n-- && *src && *dst) {
        if (*src < *dst) {
            return -1;
        } else if (*src > *dst) {
            return 1;
        }
        ++src;
        ++dst;
    }
    return 0;
}

void memcpy(void* src, void* dst, uint64_t n_bytes) {
    uint64_t n_dwords = n_bytes / 4;
    n_bytes %= 4;
    uint32_t* p = src;
    uint32_t* q = dst;
    while (n_dwords--) {
        *q++ = *p++;
    }
    uint8_t* pp = (uint8_t*)p;
    uint8_t* qq = (uint8_t*)q;
    while (n_bytes--) {
        *qq++ = *pp++;
    }
}

void memset(void* beg, uint32_t size, uint8_t val) {
    char* p = (char*)beg;
    for (int i = 0; i < size; ++i) {
        *p++ = val;
    }
}
