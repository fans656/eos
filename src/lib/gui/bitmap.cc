#include "def.h"
#include "bitmap.h"
#include "stdio.h"
#include "string.h"
#include "bmp.h"
#include "png.h"
#include "stdlib.h"

Bitmap::Bitmap(const char* path) : opaque_(true) {
    int n = strlen(path);
    const char* ext = path + n - 3;
    if (strcmp(ext, "bmp") == 0) {
        buffer_ = load_bmp(path, width_, height_);
    } else if (strcmp(ext, "png") == 0) {
        buffer_ = load_png(path, width_, height_, opaque_);
    }
    pitch_ = width_ * 4;
    own_ = true;
}

Bitmap::Bitmap(int width, int height) : opaque_(true) {
    width_ = width;
    height_ = height;
    pitch_ = width * 4;
    buffer_ = new uchar[pitch_ * height];
    own_ = true;
}

Bitmap::Bitmap(int width, int height, uchar* buffer) : opaque_(true) {
    width_ = width;
    height_ = height;
    pitch_ = width * 4;
    buffer_ = buffer;
    own_ = false;
}

void Bitmap::clear() {
    memset(buffer_, 0, pitch_ * height_);
}
