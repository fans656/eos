#include "canvas.h"
#include "widget.h"
#include "window.h"
#include "surface.h"
#include "bitmap.h"
#include "color.h"
#include "math.h"
#include "stdio.h"

inline int restricted(int x, int mi, int ma) {
    return min(max(x, mi), ma);
}

Canvas::Canvas(Widget* w) {
}

Canvas::Canvas(Window* w) {
    surface_left_ = w->margin_left();
    surface_right_ = w->frame_width() - w->margin_right();
    surface_top_ = w->margin_top();
    surface_bottom_ = w->frame_height() - w->margin_bottom();
    surface = w->surface;
    init();
}

Canvas::Canvas(Surface* surface) {
    surface_left_ = surface_top_ = 0;
    surface_right_ = surface->width;
    surface_bottom_ = surface->height;
    this->surface = surface;
    init();
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

void Canvas::draw_bitmap(Bitmap* bitmap, int x, int y) {
    draw_bitmap(bitmap, 0, 0, x, y, bitmap->width, bitmap->height);
}

void Canvas::draw_bitmap(Bitmap* bitmap, int src_x, int src_y,
        int dst_x, int dst_y, int width, int height) {
    Rect src_rc(src_x, src_y, width, height);
    Rect bitmap_rc(0, 0, bitmap->width, bitmap->height);
    src_rc.intersect(bitmap_rc);

    Rect dst_rc(dst_x, dst_y, src_rc.width(), src_rc.height());
    dst_rc.intersect(client_to_surface(src_rc));

    src_rc.set_width(dst_rc.width());
    src_rc.set_height(dst_rc.height());

    surface->blit(bitmap->buffer, bitmap->pitch,
            src_rc.left(), src_rc.top(),
            dst_rc.left(), dst_rc.top(),
            dst_rc.width(), dst_rc.height());
}

void Canvas::translate(int dx, int dy) {
    origin_top_ += dx;
    origin_left_ += dy;
}

void Canvas::client_to_surface(int& x, int& y) {
    x = restricted(x + origin_left_ + surface_left_, surface_left_, surface_right_);
    y = restricted(y + origin_top_ + surface_top_, surface_top_, surface_bottom_);
}

Rect Canvas::client_to_surface(const Rect& rc) {
    int left = rc.left();
    int top = rc.top();
    int right = rc.right();
    int bottom = rc.bottom();
    client_to_surface(left, top);
    client_to_surface(right, bottom);
    return Rect(left, top, right - left, bottom - top);
}

void Canvas::init() {
    origin_left_ = origin_top_ = 0;
    brush_color = White;
}
