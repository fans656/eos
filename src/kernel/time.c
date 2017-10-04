#include "time.h"
#include "stdio.h"
#include "process.h"

#define MAX_TIMEIT_NESTING 128

clock_t clock_counter = 0;

clock_t clock() {
    return clock_counter;
}

void clock_tick() {
    ++clock_counter;
    process_count_down();
}

void timeit(const char* fmt, ...) {
    static clock_t begs[MAX_TIMEIT_NESTING];
    static int n = 0;

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
        _printf(&fmt);
        printf(": %dms\n", (clock() - begs[n]) * PIT_MS_PRECISION);
    }
}
