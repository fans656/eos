#include "def.h"
#include "stdio.h"
#include "memory.h"
#include "filesystem.h"
#include "graphics.h"
#include "interrupt.h"
#include "elf.h"
#include "util.h"
#include "time.h"

void main() {
    init_console();
    init_memory();
    init_filesystem();
    init_graphics();
    init_interrupt();
    
    ProgEntry entry = load_elf("/art");
    uchar* stack = malloc(STACK_SIZE);
    uint esp;
    asm volatile(
            "mov eax, esp;"
            "mov %0, eax;"
            "mov esp, %1;"
            "call %2;"
            "mov eax, %0;"
            "mov esp, eax;"
            : "=m"(esp)
            : "b"(stack + STACK_SIZE), "c"(entry));
    free(stack);

    printf("kernel\n");
    
    hlt_forever();
}
