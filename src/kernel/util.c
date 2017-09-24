#include "util.h"
#include "stdio.h"

void hexdump(void* addr, uint cnt) {
    uchar* p = addr;
    for (int i = 0; i < cnt; ++i) {
        if (i % 16) {
            putchar(' ');
            if (i % 8 == 0) {
                putchar(' ');
            }
        }
        if (i && i % 16 == 0) {
            putchar('\n');
        }
        print_hex_1(*p++);
    }
    putchar('\n');
}
