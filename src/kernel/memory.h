#include "types.h"

extern uint pgdir[];

void init_memory();
void map_pages(uint pgdir[], uint vbeg, uint size);

void* malloc(size_t nbytes);
void free(void* addr);
