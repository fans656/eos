#include <stdint.h>
#include "malloc.h"
#include "io.h"
#include "util.h"

uint32_t cur = 0x100000;

void init_memory() {
}

void* malloc(uint32_t size) {
    uint32_t res = cur;
    cur += size;
    return (void*)res;
}

void free(void* p) {
}
