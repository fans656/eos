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
    surface_right_ = surface->width();
    surface_bottom_ = surface->height();
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

    left = restricted_x(left);
    right = restricted_x(right);
    top = restricted_y(top);
    bottom = restricted_y(bottom);

    surface->fill_rect(left, top, right - left, bottom - top, color);
}

void Canvas::fill_rect(const Rect& rect) {
    fill_rect(rect, brush_color);
}

void Canvas::fill_rect(const Rect& rect, uint color) {
    fill_rect(rect.left(), rect.top(), rect.width(), rect.height());
}

void Canvas::draw_bitmap(Bitmap* bitmap, int x, int y) {
    draw_bitmap(bitmap, 0, 0, x, y, bitmap->width(), bitmap->height());
}

void Canvas::draw_bitmap(Bitmap* bitmap, int src_x, int src_y,
        int dst_x, int dst_y, int width, int height) {
    Rect bitmap_rc(0, 0, bitmap->width(), bitmap->height());
    Rect rc(src_x, src_y, width, height);
    rc.intersect(bitmap_rc);
    rc.translate(dst_x, dst_y);

    rc = client_to_surface(rc);
    rc.intersect(surface_left(), surface_top(), surface_width(), surface_height());
    int new_dst_x = rc.left();
    int new_dst_y = rc.top();

    rc = surface_to_client(rc);
    rc.translate(-dst_x, -dst_y);
    rc.intersect(bitmap_rc);

    int new_src_x = rc.left();
    int new_src_y = rc.top();
    
    surface->blit(bitmap->buffer, bitmap->pitch,
            new_src_x, new_src_y,
            new_dst_x, new_dst_y,
            rc.width(), rc.height());
}

void Canvas::draw_bitmap_nocheck(Bitmap* bitmap, int src_x, int src_y,
        int dst_x, int dst_y, int width, int height) {
    surface->blit(bitmap->buffer, bitmap->pitch,
            src_x, src_y, dst_x, dst_y, width, height);
}

void Canvas::translate(int dx, int dy) {
    origin_top_ += dx;
    origin_left_ += dy;
}

void Canvas::client_to_surface(int& x, int& y) {
    x = x + origin_left_ + surface_left_;
    y = y + origin_top_ + surface_top_;
}

void Canvas::surface_to_client(int& x, int& y) {
    x = x - origin_left_ - surface_left_;
    y = y - origin_top_ - surface_top_;
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

Rect Canvas::surface_to_client(const Rect& rc) {
    int left = rc.left();
    int top = rc.top();
    int right = rc.right();
    int bottom = rc.bottom();
    surface_to_client(left, top);
    surface_to_client(right, bottom);
    return Rect(left, top, right - left, bottom - top);
}

int Canvas::restricted_x(int x) {
    return restricted(x, surface_left_, surface_right_);
}

int Canvas::restricted_y(int y) {
    return restricted(y, surface_top_, surface_bottom_);
}

void Canvas::init() {
    origin_left_ = origin_top_ = 0;
    brush_color = White;
}
