#ifndef TIME_H
#define TIME_H

#include <stdint.h>

extern uint32_t g_clock_counter;
extern uint32_t g_sleep_count;

void init_pit();
void sleep(uint32_t ms);
uint32_t clock();

#endif
