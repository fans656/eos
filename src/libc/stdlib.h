#ifndef STDLIB_H
#define STDLIB_H

#include "def.h"

void* malloc(size_t size);
void free(void* addr);
void exit(int status);

extern void _start();

#endif
