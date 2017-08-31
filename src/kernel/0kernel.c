#include "io.h"
#include "interrupt.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    setup_idt();
    clear_screen();
    
    char* s = "hello world\nthis is a timer test\n";
    for (char* p = s; *p; ++p) {
        put_char(*p);
        sleep(1);
    }

    hlt();
}
