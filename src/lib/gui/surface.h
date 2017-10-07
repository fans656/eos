#ifndef SURFACE_H
#define SURFACE_H

#include "def.h"

struct Surface {
    Surface(int width, int height, int bpp);
    ~Surface();
    
    void fill_rect(int left, int top, int width, int height, uint color);
    void draw_pixel(int x, int y, uint color);
    void blit(char* src_buffer, int pitch, int src_left,
            int src_top, int dst_left, int dst_top, int width, int height);
    
    int bpp;
    int pitch;
    int width, height;
    char* buffer;
};

#endif
