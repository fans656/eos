#include "io.h"
#include "interrupt.h"
#include "snake.h"
#include "util.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    setup_idt();
    clear_screen();
    
    printf("%p\n", key_states);
    for (int i = 0; i < 128; ++i) {
        printf("%d ", key_states[i]);
        if (i % 8 == 7) {
            put_char('\n');
        }
    }

    hlt();
}
