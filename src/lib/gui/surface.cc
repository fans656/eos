#include "surface.h"
#include "stdio.h"

Surface::Surface(int width, int height, int bpp) {
    this->bpp = bpp;
    pitch = width * bpp;
    this->width = width;
    this->height = height;
    buffer = new char[pitch * height];
}

Surface::~Surface() {
    delete[] buffer;
}

void Surface::fill_rect(int left, int top, int width, int height, uint color) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            draw_pixel(left + x, top + y, color);
        }
    }
}

void Surface::draw_pixel(int x, int y, uint color) {
    char* p = buffer + y * pitch + x * bpp;
    *p++ = color >> 16;
    *p++ = (color >> 8) & 0xff;
    *p++ = color & 0xff;
}
