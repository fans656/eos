#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

void graphic_init();
int get_screen_width();
int get_screen_height();
int get_pitch();
int get_bytes_per_pixel();
void draw_pixel(int x, int y, uint32_t color);
void fill_rect(int left, int top, int width, int height, uint32_t color);
void draw_bmp(uint8_t* bmp);

#endif
