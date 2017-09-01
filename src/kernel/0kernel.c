#include "io.h"
#include "interrupt.h"
#include "snake.h"
#include "util.h"
#include "globals.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    init_globals();
    setup_idt();
    clear_screen();
    
    snake_game();

    hlt();
}
