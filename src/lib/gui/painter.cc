#include "painter.h"
#include "bitmap.h"
#include "window.h"
#include "color.h"
#include "string.h"
#include "math.h"

constexpr int BPP = 4;

static inline uchar alpha_over(uint dst, uint src, uint alpha) {
    return (src * alpha + dst * (255 - alpha)) / 255;
}

static inline void fill_scanline(void* dst, uint color, int width) {
    asm volatile(
            "mov eax, %0;"
            "mov edi, %1;"
            "mov ecx, %2;"
            "cld;"
            "rep stosd;"
            :: "a"(color), "b"(dst), "c"(width));
}

static inline void alpha_color_over(uint& dst, uint src) {
    uchar* q = (uchar*)&dst;
    uchar* p = (uchar*)&src;
    uchar& dst_blue = *q;
    uchar& dst_green = *(q + 1);
    uchar& dst_red = *(q + 2);
    uchar& dst_alpha = *(q + 3);
    const uchar& src_blue = *p;
    const uchar& src_green = *(p + 1);
    const uchar& src_red = *(p + 2);
    const uchar& alpha = *(p + 3);
    dst_blue = alpha_over(dst_blue, src_blue, alpha);
    dst_green = alpha_over(dst_green, src_green, alpha);
    dst_red = alpha_over(dst_red, src_red, alpha);
    dst_alpha = alpha_over(dst_alpha, alpha, alpha);
}

Painter::Painter(BaseWindow* wnd)
    : bitmap(wnd->bitmap), limit_rc(wnd->client_rect_in_window_coord()) {
}

Painter::Painter(Bitmap* bitmap)
    : bitmap(bitmap), limit_rc(Rect(bitmap->rect())) {
}

Painter::Painter(Bitmap* bitmap, const Rect& rc)
    : bitmap(bitmap), limit_rc(rc) {
}

void Painter::draw_horz_line(int x1, int x2, int y) {
    if (limit_rc.top() <= y && y < limit_rc.bottom()) {
        x1 = max(x1, limit_rc.left());
        x2 = min(x2, limit_rc.right() - 1);
        uint color = pen_color;
        uint* dst = (uint*)(bitmap->buffer() + bitmap->pitch() * y + x1 * BPP);
        if (alpha_blending()) {
            for (int x = x1; x <= x2; ++x) {
                alpha_color_over(*dst++, color);
            }
        } else {
            fill_scanline(dst, color, x2 - x1 + 1);
        }
    }
}

void Painter::draw_vert_line(int y1, int y2, int x) {
    if (limit_rc.left() <= x && x < limit_rc.right()) {
        y1 = max(y1, limit_rc.top());
        y2 = min(y2, limit_rc.bottom() - 1);
        uint color = pen_color;
        int pitch = bitmap->pitch();
        uchar* dst = bitmap->buffer() + pitch * y1 + x * BPP;
        if (alpha_blending()) {
            for (int y = y1; y <= y2; ++y) {
                alpha_color_over(*(uint*)dst, color);
                dst += pitch;
            }
        } else {
            for (int y = y1; y <= y2; ++y) {
                *(uint*)dst = color;
                dst += pitch;
            }
        }
    }
}

void Painter::draw_rect(const Rect& rc) {
    draw_horz_line(rc.left(), rc.right() - 1, rc.top());
    draw_horz_line(rc.left(), rc.right() - 1, rc.bottom() - 1);
    draw_vert_line(rc.top(), rc.bottom() - 1, rc.left());
    draw_vert_line(rc.top(), rc.bottom() - 1, rc.right() - 1);
}

void Painter::fill_rect(Rect rc, uint color) {
    rc.intersect(limit_rc);
    if (alpha_blending()) {
        uint alpha = get_alpha(color);
        switch (alpha) {
            case 0x00:
                return;
            case 0xff:
                _fill_rect_solid(rc, color);
                break;
            default:
                _fill_rect_alpha(rc, color);
                break;
        }
    } else {
        _fill_rect_solid(rc, color);
    }
}

void Painter::fill_rect_solid(Rect rc, uint color) {
    rc.intersect(limit_rc);
    _fill_rect_solid(rc, color);
}

void Painter::draw_bitmap(int x, int y, Bitmap* bitmap) {
    Rect src(0, 0, bitmap->width(), bitmap->height());
    Rect dst(limit_rc.left() + x, limit_rc.top() + y,
            bitmap->width(), bitmap->height());
    draw_bitmap(dst, bitmap, src);
}

void Painter::draw_bitmap(Rect dst, Bitmap* bitmap, Rect src) {
    int dx = dst.left() - src.left();
    int dy = dst.top() - src.top();
    src.intersect(bitmap->rect());
    src.translate(dx, dy);
    src.intersect(dst);
    src.intersect(limit_rc);
    int dst_x = src.left();
    int dst_y = src.top();
    src.translate(-dx, -dy);

    if (alpha_blending() || !bitmap->opaque()) {
        alpha_blit(bitmap->buffer(), bitmap->pitch(),
                src.left(), src.top(),
                dst_x, dst_y,
                src.width(), src.height());
    } else {
        blit(bitmap->buffer(), bitmap->pitch(),
                src.left(), src.top(),
                dst_x, dst_y,
                src.width(), src.height());
    }
}

void Painter::blit(uchar* src, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    uchar* dst = bitmap->buffer();
    int dst_pitch = bitmap->pitch();
    uchar* p = src + src_top * src_pitch + src_left * BPP;
    uchar* q = dst + dst_top * dst_pitch + dst_left * BPP;
    int n = width * BPP;
    for (int i = 0; i < height; ++i) {
        memcpy(q, p, n);
        p += src_pitch;
        q += dst_pitch;
    }
}

void Painter::alpha_blit(uchar* src, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    uchar* dst = bitmap->buffer();
    int dst_pitch = bitmap->pitch();
    uchar* p = src + src_top * src_pitch + src_left * BPP;
    uchar* q = dst + dst_top * dst_pitch + dst_left * BPP;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uchar& dst_blue = *q;
            uchar& dst_green = *(q + 1);
            uchar& dst_red = *(q + 2);
            uchar& dst_alpha = *(q + 3);
            const uchar& src_blue = *p;
            const uchar& src_green = *(p + 1);
            const uchar& src_red = *(p + 2);
            const uchar& alpha = *(p + 3);
            dst_blue = alpha_over(dst_blue, src_blue, alpha);
            dst_green = alpha_over(dst_green, src_green, alpha);
            dst_red = alpha_over(dst_red, src_red, alpha);
            dst_alpha = alpha_over(dst_alpha, alpha, alpha);
            p += BPP;
            q += BPP;
        }
        p = p - width * BPP + src_pitch;
        q = q - width * BPP + dst_pitch;
    }
}

void Painter::_fill_rect_solid(const Rect& rc, uint color) {
    int pitch = bitmap->pitch();
    int width = rc.width();
    uchar* q = bitmap->buffer() + pitch * rc.top() + rc.left() * BPP;
    for (int y = rc.top(); y < rc.bottom(); ++y) {
        fill_scanline(q, color, width);
        q += pitch;
    }
}

void Painter::_fill_rect_alpha(const Rect& rc, uint color) {
    int pitch = bitmap->pitch();
    uchar* q = bitmap->buffer() + pitch * rc.top() + rc.left() * BPP;
    for (int y = rc.top(); y < rc.bottom(); ++y) {
        uint* qq = (uint*)q;
        for (int x = rc.left(); x < rc.right(); ++x) {
            alpha_color_over(*qq++, color);
        }
        q += pitch;
    }
}
