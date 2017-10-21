#ifndef CANVAS_H
#define CANVAS_H

#include "def.h"
#include "point.h"
#include "rect.h"

//struct Widget;
//struct Window;
//struct Surface;
//struct Bitmap;
//
//struct Canvas {
//    Canvas(Widget* w);
//    Canvas(Window* w);
//    Canvas(Surface* surface);
//
//    void draw_line(int x1, int y1, int x2, int y2, int width, uint color);
//    void draw_circle(Point center, int radius);
//    
//    void fill_rect(int left, int top, int width, int height);
//    void fill_rect(int left, int top, int width, int height, uint color);
//    void fill_rect(const Rect& rect);
//    void fill_rect(const Rect& rect, uint color);
//    
//    void draw_rect(int left, int top, int width, int height, uint color);
//    
//    void draw_bitmap(Bitmap* bitmap, int x, int y);
//    void draw_bitmap(Bitmap* bitmap, int src_x, int src_y,
//            int dst_x, int dst_y, int width, int height);
//    void draw_bitmap_nocheck(Bitmap* bitmap, int src_x, int src_y,
//            int dst_x, int dst_y, int width, int height);
//
//    void draw_text(int x, int y, const char* text);
//    
//    void blit(Surface* src, int x, int y, int width, int height);
//    void alpha_blit(Surface* src, int x, int y, int width, int height);
//
//    void translate(int dx, int dy);
//    
//    void save();
//    void restore();
//    
//    void init();
//
//    int restricted_x(int x);
//    int restricted_y(int y);
//
//    void client_to_surface(int& x, int& y);
//    Rect client_to_surface(const Rect& rc);
//
//    void surface_to_client(int& x, int& y);
//    Rect surface_to_client(const Rect& rc);
//    
//    inline int surface_left() const { return surface_left_; }
//    inline int surface_top() const { return surface_top_; }
//    inline int surface_right() const { return surface_right_; }
//    inline int surface_bottom() const { return surface_bottom_; }
//    inline int surface_width() const { return surface_right() - surface_left(); }
//    inline int surface_height() const { return surface_bottom() - surface_top(); }
//    
//    int origin_left_, origin_top_;
//    int surface_left_, surface_top_, surface_right_, surface_bottom_;
//    uint brush_color;
//    Surface* surface;
//};

#endif
