#include "io.h"
#include "interrupt.h"
#include "snake.h"
#include "util.h"
#include "draw.h"
#include "image.h"

void print_sector(int i_sector);

void kernel_entry() {
    setup_idt();
    graphic_init();

    int width = *(uint32_t*)0x20000;
    int height = *(uint32_t*)0x20004;
    
    int bytes_per_pixel = get_bytes_per_pixel();
    
    int left = (get_screen_width() - width) / 2;
    int top = (get_screen_height() - height) / 2;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint32_t offset = 0x20008 + (y * width + x) * bytes_per_pixel;
            uint32_t color = *(uint32_t*)offset & 0x00ffffff;
            draw_pixel(left + x, top + y, color);
        }
    }

    //snake_game();
    hlt();
}
