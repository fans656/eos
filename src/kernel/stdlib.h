#ifndef STDLIB_H
#define STDLIB_H

#include "def.h"

#define RAND_MAX 32768

int rand();
void srand(uint seed);

int randint(int min, int max);

#endif
