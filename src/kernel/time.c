#include "time.h"

uint32_t g_clock_counter = 0;

uint32_t clock() {
    return g_clock_counter;
}
