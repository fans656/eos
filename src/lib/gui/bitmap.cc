#include "def.h"
#include "bitmap.h"
#include "stdio.h"
#include "string.h"

typedef struct __attribute__((packed)) {
    char signature[2];
    uint file_size;
    uint _reserved;
    uint offset;
} BitmapHeader;

typedef struct __attribute__((packed)) {
    uint header_size;
    uint width;
    uint height;
    ushort planes;
    ushort bpp;
    uint compression;
    uchar _notcare[20];
} BitmapInfoHeader;

#define BMP_INFO_HEADER(bmp) ((BitmapInfoHeader*)((char*)(bmp) + sizeof(BitmapHeader)))
#define BMP_HEADER(bmp) ((BitmapHeader*)(bmp))
#define bmp_width(bmp) (BMP_INFO_HEADER((bmp))->width)
#define bmp_height(bmp) (BMP_INFO_HEADER((bmp))->height)
#define bmp_data(bmp) ((char*)(bmp) + BMP_HEADER((bmp))->offset)
#define bmp_bpp(bmp) (BMP_INFO_HEADER((bmp))->bpp / 8)

static int bmp_pitch(void* bmp) {
    BitmapInfoHeader* bih = BMP_INFO_HEADER(bmp);
    return align4(bih->width * bih->bpp / 8);
}

Bitmap::Bitmap(const char* path) {
    char* bmp = (char*)load_file(path);
    int width = width_ = bmp_width(bmp);
    int height = height_ = bmp_height(bmp);
    pitch = width * 4;
    buffer = new char[width * height * 4];

    char* src_data = bmp_data(bmp);
    int src_pitch = bmp_pitch(bmp);
    uint* q = (uint*)buffer;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uchar* p = (uchar*)src_data + (height - 1 - y) * src_pitch + x * 3;
            uint color = *p | (*(p + 1) << 8) | (*(p + 2) << 16);
            *q++ = color;
        }
    }
    delete bmp;
}
