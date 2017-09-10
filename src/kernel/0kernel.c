#include "io.h"
#include "util.h"
#include "draw.h"
#include "interrupt.h"
#include "filesystem.h"

void main();

void kernel_entry() {
    clear_screen();
    setup_idt();
    init_filesystem();
    main();
    hlt();
}

void main() {
    tree();
}
