#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "def.h"

void init_graphics();

int get_screen_width();
int get_screen_height();

int get_pitch();
int get_bytes_per_pixel();

void screen_fill_black();

void draw_pixel(int x, int y, uint color);
void fill_rect(int left, int top, int width, int height, uint color);

void draw_bmp(char* fpath);
void draw_bmp_at(const char* fpath, int x, int y);

void bmp_blit_nocheck(void* bmp, int src_left, int src_top,
        int dst_left, int dst_top, int width, int height);
void bmp_blit(void* bmp, int src_left, int src_top,
        int dst_left, int dst_top, int width, int height);
void bmp_draw_at(void* bmp, int left, int top);

void draw_char(char ch, int row, int col);

void sync_console_at(int row, int col);
void sync_console();

#endif
