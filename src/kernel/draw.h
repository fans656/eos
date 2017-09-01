#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

void draw_pixel(int x, int y, uint8_t color);
void fill_rect(int left, int top, int w, int h, uint8_t color);

#endif
