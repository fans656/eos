#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include "util.h"
#include "constants.h"

#define assert(val) if (!(val)) panic("Assertion failed: %s, line %d, %s\n", __FILE__, __LINE__, __func__)

void panic(char* fmt, ...);

extern int sleep_count;

void hlt();

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);

uint16_t inw(uint16_t port);
void outw(uint16_t port, uint16_t val);

void srand(uint32_t seed);
uint32_t rand();
int randint(int min, int max);

int strcmp(char* src, char* dst);
int strncmp(char* src, char* dst, int n);

void memcpy(void* src, void* dst, uint64_t n_bytes);
void memset(void* beg, uint32_t size, uint8_t val);
void strcpy(char* src, char* dst);

#endif
