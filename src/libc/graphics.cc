#include "graphics.h"

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

#define align4(x) (((x) + 3) / 4 * 4)

int bmp_pitch(void* bmp) {
    BitmapInfoHeader* bih = BMP_INFO_HEADER(bmp);
    return align4(bih->width * bih->bpp / 8);
}

void bmp_blit(void* bmp, int src_left, int src_top,
        int dst_left, int dst_top, int width, int height) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_BMP_BLIT));
}

void draw_bmp_at(void* bmp, int left, int top) {
    bmp_blit(bmp, 0, 0, left, top, bmp_width(bmp), bmp_height(bmp));
}
