#ifndef SURFACE_H
#define SURFACE_H

#include "def.h"

struct Surface {
    Surface(int width, int height, int bpp);
    ~Surface();
    
    inline int width() const { return width_; }
    inline int height() const { return height_; }
    inline int pitch() const { return pitch_; }
    inline uchar* buffer() const { return dst; }
    
    void fill_rect(int left, int top, int width, int height, uint color);
    void draw_pixel(int x, int y, uint color);
    void blit(uchar* src, int src_pitch, int src_left, int src_top,
            int dst_left, int dst_top, int width, int height);
    void alpha_blit(uchar* src, int src_pitch, int src_left, int src_top,
            int dst_left, int dst_top, int width, int height);
    
    int bpp;
    int pitch_;
    int width_, height_;
    uchar* dst;
};

#endif
