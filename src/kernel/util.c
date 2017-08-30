#include <stdint.h>

void hlt() {
    while (1) {
        ;
    }
}

void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" :: "a"(val), "dN"(port));
}
