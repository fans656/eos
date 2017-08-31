#include "io.h"
#include "interrupt.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    setup_idt();
    asm volatile ("int 49");

    //clear_screen();
    //asm volatile ("int 49");
    print_str(CONSOLE_PROMPT);
    //while (1) {
    //    char ch = get_char();
    //    put_char(ch);
    //}
    hlt();
}
