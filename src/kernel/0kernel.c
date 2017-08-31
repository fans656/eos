#include "io.h"
#include "interrupt.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    clear_screen();
    setup_idt();

    //uint8_t x = inb(0x21);
    //print_byte(x);
    //put_char('\n');
    //uint8_t y = inb(0xa1);
    //print_byte(y);

    hlt();
}
