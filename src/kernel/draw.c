#include "draw.h"

uint8_t* GUI_VIDEO_MEM = (uint8_t*)0xa0000;

void draw_pixel(int x, int y, uint8_t color) {
    GUI_VIDEO_MEM[y * 320 + x] = color;
}

void fill_rect(int left, int top, int w, int h, uint8_t color) {
    for (int y = top; y < top + h; ++y) {
        for (int x = left; x < left + w; ++x) {
            GUI_VIDEO_MEM[y * 320 + x] = color;
        }
    }
}
