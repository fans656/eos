#ifndef TIME_H
#define TIME_H

#include "def.h"

#define PIT_BASE_FREQUENCY 1193182
#define PIT_MS_PRECISION 10

typedef uint clock_t;

clock_t clock();
extern "C" void clock_tick();

void _timeit(const char** pfmt);
void timeit(const char* fmt, ...);

#endif
