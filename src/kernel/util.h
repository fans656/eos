#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include "util.h"

#define RAND_MAX 65536

extern int sleep_count;

void hlt();
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);
void sleep(int cnt);
void srand(uint32_t seed);
uint32_t rand();
int randint(int min, int max);

#endif
