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

uint8_t* bmp = (uint8_t*)(1 * MB);

void main() {
    init_disk();
    graphic_init();
    
    int i_sector = 1 * MB / 512;
    for (int i = 0; i < 4 * MB / 512; ++i) {
        read_sector(i_sector++, bmp + i * 512);
    }
    draw_bmp(bmp);
}
