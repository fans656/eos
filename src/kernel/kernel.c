#include <stdint.h>

void main() {
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 80; ++j) {
            *(uint16_t*)(0xb8000 + (i * 80 + j) * 2) = 0x0300 | '.';
        }
    }
    asm("hlt");
}
