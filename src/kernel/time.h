#ifndef TIME_H
#define TIME_H

#include "def.h"

#define PIT_BASE_FREQUENCY 1193182
#define PIT_MS_PRECISION 10

typedef uint clock_t;

extern "C" {
clock_t clock();
void clock_tick();

void timeit(const char* fmt, ...);
}

#endif
