#include "def.h"

extern uint pgdir[];

void init_memory();
void map_pages(uint pgdir[], uint vbeg, uint size);

void* named_malloc(size_t size, const char* name);
void* malloc(size_t nbytes);
void free(void* addr);

void memory_test();
