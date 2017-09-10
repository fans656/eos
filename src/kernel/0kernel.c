#include "io.h"
#include "interrupt.h"
#include "snake.h"
#include "util.h"
#include "draw.h"
#include "image.h"
#include "disk.h"

void main();

void kernel_entry() {
    clear_screen();
    setup_idt();
    main();
    hlt();
}

void main() {
    init_disk();
    
    char buffer[512];
    int i_sector = 0x106000 / 512;
    //i_sector += 8 * 2;
    printf("read %d sector\n", i_sector);
    read_sector(i_sector, buffer);
    print_mem(buffer, 256);
}
