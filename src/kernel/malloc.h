#ifndef MALLOC_H
#define MALLOC_H

#include "def.h"

void init_malloc();
void* kmalloc(size_t size);
void kfree(void* addr);

#endif
