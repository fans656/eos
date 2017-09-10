#ifndef MALLOC_H
#define MALLOC_H

void init_memory();

void* malloc(uint32_t size);
void free(void* p);

#endif
