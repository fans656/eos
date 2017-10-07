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
    pitch = bmp_pitch(bmp);
    width = bmp_width(bmp);
    height = bmp_height(bmp);
    buffer = new char[pitch * height];

    char* p = bmp_data(bmp) + pitch * (height - 1);
    char* q = buffer;
    for (int y = 0; y < height; ++y) {
        memcpy(q, p, pitch);
        q += pitch;
        p -= pitch;
    }
    delete bmp;
}
