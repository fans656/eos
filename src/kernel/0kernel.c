#include "io.h"
#include "util.h"
#include "draw.h"
#include "interrupt.h"
#include "filesystem.h"
#include "malloc.h"
#include "constants.h"

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
    FILE* fp = fopen("/a.img");
    uint64_t size = fp->entry->size;
    char* data = (char*)0xf00000;
    fread(fp, size, data);
    asm("call 0xf00000");
}
