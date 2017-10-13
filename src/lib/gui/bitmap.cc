#include "def.h"
#include "bitmap.h"
#include "stdio.h"
#include "string.h"
#include "bmp.h"
#include "png.h"
#include "stdlib.h"

Bitmap::Bitmap(const char* path) {
    int n = strlen(path);
    const char* ext = path + n - 3;
    if (strcmp(ext, "bmp") == 0) {
        buffer = load_bmp(path, width_, height_);
    } else if (strcmp(ext, "png") == 0) {
        buffer = load_png(path, width_, height_);
    }
    pitch_ = width_ * 4;
}
