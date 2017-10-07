#ifndef SURFACE_H
#define SURFACE_H

#include "def.h"

struct Surface {
    Surface(int width, int height, int bpp);
    ~Surface();
    
    void fill_rect(int left, int top, int width, int height, uint color);
    void draw_pixel(int x, int y, uint color);
    
    int bpp;
    int pitch;
    int width, height;
    char* buffer;
};

#endif
