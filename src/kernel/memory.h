#include "types.h"

extern uint pgdir[];

void init_memory();
void map_pages(uint pgdir[], uint vbeg, uint size);
