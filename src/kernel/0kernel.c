#include "io.h"
#include "util.h"
#include "draw.h"
#include "interrupt.h"
#include "filesystem.h"
#include "malloc.h"
#include "constants.h"
#include "loader.h"

void main();

void kernel_entry() {
    asm volatile (
            "mov ax, 16;"
            "mov ds, ax;"
            "mov es, ax;"
            "mov ss, ax;"
            "mov ebp, 0;"
            "mov esp, %0;" :: "i"(STACK_END)
            );
    clear_screen();
    setup_idt();
    init_memory();
    graphic_init();
    init_filesystem();
    main();
    hlt();
}

void main() {
    execute("/bin/ls");
}
