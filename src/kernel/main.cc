#include "stdio.h"
#include "interrupt.h"
#include "memory.h"
#include "filesystem.h"
#include "graphics.h"
#include "process.h"
#include "util.h"
#include "keyboard.h"
#include "message.h"

extern "C" void main() {
    init_console();
    init_interrupt();
    init_memory();

    init_filesystem();
    init_keyboard();
    init_graphics();
    init_process();
    init_message();

    asm("sti");
    while (true) {
        //gui_work();
        process_release();
        asm("hlt");
    }
}
