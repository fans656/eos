#include "io.h"
#include "interrupt.h"
#include "snake.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    setup_idt();
    clear_screen();
    
    snake_game();

    hlt();
}
