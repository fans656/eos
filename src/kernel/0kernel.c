#include "io.h"
#include "interrupt.h"
#include "snake.h"
#include "util.h"
#include "draw.h"
#include "image.h"

void kernel_entry() {
    int margin_top = (200 - (200 / 32 * 32)) / 2;
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 10; ++col) {
            int base_y = row * 32 + margin_top;
            int base_x = col * 32;
            for (int y = 0; y < 32; ++y) {
                for (int x = 0; x < 32; ++x) {
                    draw_pixel(base_x + x, base_y + y, image[y * 32 + x]);
                }
            }
        }
    }

    //setup_idt();
    //clear_screen();
    //
    //snake_game();

    hlt();
}
