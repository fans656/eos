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
        //debug("%d\n", malloc_list_size());
        process_release();
        while (process_is_idle()) {
            asm("hlt");
        }
        process_yield();
    }
}
