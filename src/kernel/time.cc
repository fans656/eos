#include "time.h"
#include "stdio.h"
#include "process.h"

#define MAX_TIMEIT_NESTING 128

clock_t current_ticks = 0;

clock_t clock() {
    return current_ticks;
}

void clock_tick() {
    ++current_ticks;
    process_count_down();
}

void _timeit(const char** pfmt) {
    static clock_t begs[MAX_TIMEIT_NESTING];
    static int n = 0;
    const char* fmt = *pfmt;

    if (!fmt) {
        if (n == MAX_TIMEIT_NESTING) {
            panic("MAX_TIMEIT_NESTING");
        }
        begs[n++] = clock();
    } else {
        if (n == 0) {
            panic("un-paired timeit call");
        }
        --n;
        for (int i = 0; i < n; ++i) {
            printf("  ");
        }
        _printf(pfmt);
        printf(": %dms\n", (clock() - begs[n]) * PIT_MS_PRECISION);
    }
}

void timeit(const char* fmt, ...) {
    _timeit(&fmt);
}

void test_loop_count_between_timer_interrupt() {
    asm("sti");
    uint max = 0;
    while (true) {
        uint cnt = 0;
        uint old = current_ticks;
        while (current_ticks == old) {
            ++cnt;
        }
        if (cnt > max) {
            max = cnt;
            printf("Max %d loop count between two timer interrupts\n", max);
        }
    }
}
