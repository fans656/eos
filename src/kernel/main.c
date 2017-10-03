#include "stdio.h"
#include "interrupt.h"
#include "memory.h"
#include "filesystem.h"
#include "graphics.h"
#include "process.h"
#include "util.h"

void main() {
    init_console();
    init_interrupt();
    init_memory();
    init_filesystem();
    //init_graphics();
    init_process();

    asm("sti");
    while (true) {
        asm("hlt");
    }
}
