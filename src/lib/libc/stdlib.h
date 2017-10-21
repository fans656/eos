#ifndef STDLIB_H
#define STDLIB_H

#include "../common/def.h"

#define RAND_MAX 32768

void* malloc(size_t size);
void free(void* addr);
void exit(int status);

extern "C" void _start();

int rand();
void srand(uint seed);
int randint(int min, int max);

#endif
