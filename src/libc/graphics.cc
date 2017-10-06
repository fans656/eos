#include "graphics.h"

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

void memory_blit(
        const char* buffer, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_MEMORY_BLIT));
}
