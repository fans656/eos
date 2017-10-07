#ifndef CANVAS_H
#define CANVAS_H

#include "def.h"
#include "point.h"
#include "rect.h"

struct Widget;
struct Window;
struct Surface;
struct Bitmap;

struct Canvas {
    Canvas(Widget* w);
    Canvas(Window* w);
    

    void draw_line(int x1, int y1, int x2, int y2, int width, uint color);
    void draw_circle(Point center, int radius);
    
    void fill_rect(int left, int top, int width, int height);
    void fill_rect(int left, int top, int width, int height, uint color);
    void fill_rect(const Rect& rect);
    void fill_rect(const Rect& rect, uint color);
    
    void draw_rect(int left, int top, int width, int height, uint color);
    
    void draw_bitmap(Bitmap* bitmap);

    void draw_text(int x, int y, const char* text);

    
    void translate(int dx, int dy);
    
    void save();
    void restore();
    
    void client_to_surface(int& x, int& y);
    
    int origin_left_, origin_top_;
    int surface_left_, surface_top_, surface_right_, surface_bottom_;
    uint brush_color;
    Surface* surface;
};

#endif
