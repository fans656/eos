#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../common/def.h"

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

int bmp_pitch(void* bmp);

void bmp_blit(void* bmp, int src_left, int src_top,
        int dst_left, int dst_top, int width, int height);
void draw_bmp_at(void* bmp, int left, int top);

void memory_blit(const char* buffer, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height);

#endif
