#include <stdint.h>
#include "util.h"

#define MS_PER_IRQ 54.9254

uint32_t g_current_random_value = 0;

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

int sleep_count = -1;

void sleep(int ms) {
    if (ms < MS_PER_IRQ) {
        ms = MS_PER_IRQ + 1;
    }
    sleep_count = (int)(ms / MS_PER_IRQ + 0.5);
    while (sleep_count) {
        asm("hlt");
    }
}

void srand(uint32_t seed) {
    g_current_random_value = seed;
    rand();
}

uint32_t rand() {
    g_current_random_value = (1238417 * g_current_random_value + 58262361) % RAND_MAX;
    return g_current_random_value;
}

int randint(int min, int max) {
    return min + rand() % (max - min + 1);
}
