#include "io.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    clear_screen();
    print_str(CONSOLE_PROMPT);
    while (1) {
        char ch = get_char();
        put_char(ch);
    }
    hlt();
}
