#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include "util.h"

#define RAND_MAX 65536

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

#define panic hlt

extern int sleep_count;

void hlt();

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);

uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t val);

void sleep(int cnt);
void srand(uint32_t seed);
uint32_t rand();
int randint(int min, int max);

void memcpy(void* src, void* dst, uint64_t n_bytes);
int strncmp(char* src, char* dst, int n);

#endif
