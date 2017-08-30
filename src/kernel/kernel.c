#include "textmode.h"
#include "util.h"
#include "keyboard.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    clear_screen();
    while (1) {
        uint8_t status = inb(0x64);
        if (status & 0x1) {
            uint8_t scancode = inb(0x60);
            char ch = SCANCODE_TO_KEY[scancode];
            put_char(ch);
            print_str(" ");
            print_str(scancode & 0x80 ? "Up" : "Down");
            print_str("\n");
        }
    }
    hlt();
}
