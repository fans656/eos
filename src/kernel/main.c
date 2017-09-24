#include "vm.h"
#include "stdio.h"

void main() {
    clear_screen();
    init_vm();
    
    printf("hello eos!\n");

    asm("hlt");
}
