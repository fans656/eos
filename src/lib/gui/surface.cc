#include "surface.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

Surface::Surface(int width, int height, int bpp) {
    this->bpp = bpp;
    pitch_ = align4(width * bpp);
    width_ = width;
    height_ = height;
    dst = new uchar[pitch_ * height];
    alt_dst = new uchar[pitch_ * height];
}

Surface::~Surface() {
    delete[] dst;
    delete[] alt_dst;
}

void Surface::fill_rect(int left, int top, int width, int height, uint color) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            draw_pixel(left + x, top + y, color);
        }
    }
}

void Surface::draw_pixel(int x, int y, uint color) {
    *(uint*)(dst + y * pitch_ + x * bpp) = color;
}

void Surface::blit(uchar* src, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    uchar* p = src + src_top * src_pitch + src_left * bpp;
    uchar* q = dst + dst_top * pitch_ + dst_left * bpp;
    int n = width * bpp;
    for (int i = 0; i < height; ++i) {
        memcpy(q, p, n);
        p += src_pitch;
        q += pitch_;
    }
}

static inline uchar alpha_over(uint dst, uint src, uint alpha) {
    return (src * alpha + dst * (255 - alpha)) / 255;
}

void Surface::alpha_blit(uchar* src, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    uchar* p = src + src_top * src_pitch + src_left * bpp;
    uchar* q = dst + dst_top * pitch_ + dst_left * bpp;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uchar& dst_blue = *q;
            uchar& dst_green = *(q + 1);
            uchar& dst_red = *(q + 2);
            const uchar& src_blue = *p;
            const uchar& src_green = *(p + 1);
            const uchar& src_red = *(p + 2);
            const uchar& alpha = *(p + 3);
            dst_blue = alpha_over(dst_blue, src_blue, alpha);
            dst_green = alpha_over(dst_green, src_green, alpha);
            dst_red = alpha_over(dst_red, src_red, alpha);
            p += bpp;
            q += bpp;
        }
        p = p - width * bpp + src_pitch;
        q = q - width * bpp + pitch_;
    }
}
