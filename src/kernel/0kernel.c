#include "io.h"
#include "interrupt.h"
#include "snake.h"
#include "util.h"

char* CONSOLE_PROMPT = "$ ";

void kernel_entry() {
    setup_idt();
    clear_screen();
    
    printf("x = %d, y = %d\n%s %c %f", 3, 5, "really?", 'A');

    hlt();
}
