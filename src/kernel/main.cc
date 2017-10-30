#include "stdio.h"
#include "interrupt.h"
#include "memory.h"
#include "filesystem.h"
#include "graphics.h"
#include "process.h"
#include "util.h"
#include "keyboard.h"
#include "message.h"
#include "dict.h"
#include "bmp.h"
#include "math.h"
#include "time.h"
#include "asm.h"

extern "C" void main() {
    init_console();     // clear screen
    init_interrupt();   // early interrupt to catch page faults
    init_memory();      // malloc/free
    init_filesystem();  // fopen/fread etc
    init_keyboard();
    init_graphics();    // screen info, font, mouse etc
    init_message();     // IPC
    init_process();     // process will only be started by the first timer interrupt

    //debug_on();

    asm("sti");  // Open interrupt, from now on the execution may be switch to another process.
                 // Here as the kernel process will only do some cleaning up work.

    while (true) {
        process_release();
        process_start();
        while (process_is_idle()) {
            asm("hlt");
        }
        process_yield();
    }
}
