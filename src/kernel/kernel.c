#include "textmode.h"
#include "util.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    clear_screen();
    print_str(CONSOLE_PROMPT);
    hlt();
}
