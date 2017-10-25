#ifndef _PAINTER_H
#define _PAINTER_H

#include "def.h"
#include "rect.h"

struct BaseWindow;
struct Bitmap;

struct Painter {
    Painter(BaseWindow* wnd);
    Painter(Bitmap* bitmap);
    Painter(Bitmap* bitmap, const Rect& rc);
    
    void draw_horz_line(int x1, int x2, int y);
    void draw_vert_line(int y1, int y2, int x);

    void draw_rect(const Rect& rc);

    void fill_rect(Rect rc, uint color);
    void fill_rect_solid(Rect rc, uint color);

    void draw_bitmap(int x, int y, Bitmap* bitmap);
    void draw_bitmap(Rect dst, Bitmap* bitmap, Rect src);
    
    void set_pen_color(uint color) { pen_color = color; }

    bool alpha_blending() const { return alpha_blending_; }
    void set_alpha_blending(bool on) { alpha_blending_ = on; }

    void blit(uchar* src, int src_pitch, int src_left, int src_top,
            int dst_left, int dst_top, int width, int height);
    void alpha_blit(uchar* src, int src_pitch, int src_left, int src_top,
            int dst_left, int dst_top, int width, int height);

    void _fill_rect_solid(const Rect& rc, uint color);
    void _fill_rect_alpha(const Rect& rc, uint color);
    
    Bitmap* bitmap;
    Rect limit_rc;
    uint pen_color = 0;
    bool alpha_blending_ = false;
};

#endif
