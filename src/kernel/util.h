#ifndef UTIL_H
#define UTIL_H

#include "def.h"

static inline void hlt_forever() {
    while (true) {
        asm("hlt");
    }
}

void hexdump(void* addr, uint cnt);

#endif
