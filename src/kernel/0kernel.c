#include "io.h"
#include "util.h"
#include "draw.h"
#include "interrupt.h"
#include "filesystem.h"
#include "malloc.h"
#include "constants.h"
#include "loader.h"
#include "time.h"

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
    init_pit();
    setup_idt();
    init_memory();
    graphic_init();
    init_filesystem();
    main();
    hlt();
}

void main() {
    for (int i = 0;; ++i) {
        printf("%d\n", i);
        sleep(1000);
    }
}
