#include <stdint.h>

uint16_t* VIDEO_MEM = (uint16_t*)0xb8000;

void clear_screen();
void hlt();

void kernel_entry() {
    char* s = "Hello eos! This is from C.";
    clear_screen();
    for (int i = 0; s[i]; ++i) {
        VIDEO_MEM[i] = 0x0f00 | s[i];
    }
    hlt();
}

void clear_screen() {
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 80; ++j) {
            VIDEO_MEM[i * 80 + j] = 0x0f00 | ' ';
        }
    }
}

void hlt() {
    while (1) {
        ;
    }
}
