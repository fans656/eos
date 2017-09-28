#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"

void init_graphics();

int get_screen_width();
int get_screen_height();

int get_pitch();
int get_bytes_per_pixel();

void screen_fill_black();

void draw_pixel(int x, int y, uint color);
void fill_rect(int left, int top, int width, int height, uint color);

void draw_bmp(char* fpath);
void draw_bmp_at(char* fpath, int x, int y);

void draw_char(char ch, int row, int col);

void sync_console();

#endif
