#include "bmp.h"
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

#define BMP_INFO_HEADER(bmp) ((BitmapInfoHeader*)((uchar*)(bmp) + sizeof(BitmapHeader)))
#define BMP_HEADER(bmp) ((BitmapHeader*)(bmp))
#define bmp_width(bmp) (BMP_INFO_HEADER((bmp))->width)
#define bmp_height(bmp) (BMP_INFO_HEADER((bmp))->height)
#define bmp_data(bmp) ((uchar*)(bmp) + BMP_HEADER((bmp))->offset)
#define bmp_bpp(bmp) (BMP_INFO_HEADER((bmp))->bpp / 8)

static int bmp_pitch(void* bmp) {
    BitmapInfoHeader* bih = BMP_INFO_HEADER(bmp);
    return align4(bih->width * bih->bpp / 8);
}

uchar* load_bmp(const char* path, int& width, int& height) {
    uchar* bmp = (uchar*)load_file(path);
    width = bmp_width(bmp);
    height = bmp_height(bmp);
    uchar* data = bmp_data(bmp);
    int pitch = bmp_pitch(bmp);
    uint* res = new uint[width * height];
    uint* q = res;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uchar* p = (uchar*)data + (height - 1 - y) * pitch + x * 3;
            uint color = *p | (*(p + 1) << 8) | (*(p + 2) << 16);
            *q++ = color;
        }
    }
    delete bmp;
    return (uchar*)res;
}
