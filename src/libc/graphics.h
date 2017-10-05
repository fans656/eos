#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../common/def.h"

#define bmp_width(bmp) (BMP_INFO_HEADER((bmp))->width)
#define bmp_height(bmp) (BMP_INFO_HEADER((bmp))->height)
#define bmp_data(bmp) ((char*)(bmp) + BMP_HEADER((bmp))->offset)
#define bmp_bpp(bmp) (BMP_INFO_HEADER((bmp))->bpp / 8)

int bmp_pitch(void* bmp);

void bmp_blit(void* bmp, int src_left, int src_top,
        int dst_left, int dst_top, int width, int height);
void draw_bmp_at(void* bmp, int left, int top);

#endif
