#include "canvas.h"
#include "widget.h"
#include "window.h"
#include "surface.h"
#include "color.h"
#include "math.h"
#include "stdio.h"

inline int restricted(int x, int mi, int ma) {
    return min(max(x, mi), ma);
}

Canvas::Canvas(Widget* w) {
}

Canvas::Canvas(Window* w) {
    origin_left_ = origin_top_ = 0;
    surface_left_ = w->margin_left();
    surface_right_ = w->frame_width() - w->margin_right();
    surface_top_ = w->margin_top();
    surface_bottom_ = w->frame_height() - w->margin_bottom();
    surface = w->surface;
    brush_color = White;
}

void Canvas::fill_rect(int left, int top, int width, int height) {
    fill_rect(left, top, width, height, brush_color);
}

void Canvas::fill_rect(int left, int top, int width, int height, uint color) {
    int right = left + width;
    int bottom = top + height;
    client_to_surface(left, top);
    client_to_surface(right, bottom);
    surface->fill_rect(left, top, right - left, bottom - top, color);
}

void Canvas::fill_rect(const Rect& rect) {
    fill_rect(rect, brush_color);
}

void Canvas::fill_rect(const Rect& rect, uint color) {
    fill_rect(rect.left(), rect.top(), rect.width(), rect.height());
}

void Canvas::translate(int dx, int dy) {
    origin_top_ += dx;
    origin_left_ += dy;
}

void Canvas::client_to_surface(int& x, int& y) {
    x = restricted(x + origin_left_ + surface_left_, surface_left_, surface_right_);
    y = restricted(y + origin_top_ + surface_top_, surface_top_, surface_bottom_);
}
