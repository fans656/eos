/*
http://wiki.osdev.org/PIT
 */
#include "time.h"

#define PIT_BASE_FREQUENCY 1193182
#define PIT_MS_PRECISION 10

uint32_t g_clock_counter = 1;
uint32_t g_sleep_count = 0;

void init_pit() {
    uint16_t val = PIT_MS_PRECISION * PIT_BASE_FREQUENCY / 1000;
    asm(
            "mov ax, %0;"
            "mov ax, 0x2e9c;"
            "out 0x40, al;"
            "rol ax, 8;"
            "out 0x40, al;" :: "m"(val)
       );
}

void sleep(uint32_t ms) {
    if (!ms) {
        return;
    }
    if (ms < PIT_MS_PRECISION) {
        ms = PIT_MS_PRECISION;
    }
    g_sleep_count = ms / PIT_MS_PRECISION;
    while (g_sleep_count) {
        asm("hlt");
    }
}

uint32_t clock() {
    return g_clock_counter;
}
