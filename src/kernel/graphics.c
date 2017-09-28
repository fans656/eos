#include "graphics.h"
#include "conf.h"
#include "filesystem.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"
#include "math.h"

// http://www.delorie.com/djgpp/doc/rbinter/ix/10/4F.html

typedef struct  __attribute__ ((packed)) {
   ushort attributes;  // deprecated
   uchar window_a;    // deprecated
   uchar window_b;   // deprecated
   ushort granularity;  // deprecated
   ushort window_size;
   ushort segment_a;
   ushort segment_b;
   uint win_func_ptr;  // deprecated
   ushort pitch;      
   ushort width;     
   ushort height;   
   uchar w_char;   
   uchar y_char;  
   uchar planes;
   uchar bpp;    
   uchar banks; 
   uchar memory_model;
   uchar bank_size;
   uchar image_pages;
   uchar reserved0;

   uchar red_mask;
   uchar red_position;
   uchar green_mask;
   uchar green_position;
   uchar blue_mask;
   uchar blue_position;
   uchar reserved_mask;
   uchar reserved_position;
   uchar direct_color_attributes;

   uint framebuffer;      
   uint off_screen_mem_off;
   ushort off_screen_mem_size;
   uchar reserved1[206];
} vbe_mode_info_structure;

static vbe_mode_info_structure* mode_info = (vbe_mode_info_structure*)(0x500 + KERNEL_BASE);
static uchar* graphic_video_mem;
static int screen_width;
static int screen_height;
static int pitch;
static int bytes_per_pixel;

void init_graphics() {
    graphic_video_mem = (uchar*)mode_info->framebuffer;
    screen_width = mode_info->width;
    screen_height = mode_info->height;
    pitch = mode_info->pitch;
    bytes_per_pixel = mode_info->bpp / 8;

    uint beg = (uint)graphic_video_mem;
    for (size_t sz = 0; sz < 16 * MB; sz += 4 * MB) {
        pgdir[beg >> 22] = beg | PTE_P | PTE_W | PTE_PS;
        beg += 4 * MB;
    }
    
    printf("graphic_video_mem: %x\n", graphic_video_mem);
    printf("width: %d, height: %d\n", screen_width, screen_height);
    printf("pitch: %d, bytes_per_pixel: %d\n", pitch, bytes_per_pixel);
}

int get_screen_width() {
    return screen_width;
}

int get_pitch() {
    return pitch;
}

int get_bytes_per_pixel() {
    return bytes_per_pixel;
}

int get_screen_height() {
    return screen_height;
}

void draw_pixel(int x, int y, uint color) {
    uint i = y * pitch + x * bytes_per_pixel;
    graphic_video_mem[i] = color & 0xff;
    graphic_video_mem[i + 1] = (color >> 8) & 0xff;
    graphic_video_mem[i + 2] = (color >> 16) & 0xff;
}

void fill_rect(int left, int top, int width, int height, uint color) {
    for (int y = top; y < top + height; ++y) {
        for (int x = left; x < left + width; ++x) {
            draw_pixel(x, y, color);
        }
    }
}

void screen_fill_black() {
    int bpp = get_bytes_per_pixel();
    int width = get_screen_width();
    int height = get_screen_height();
    uint n_bytes = width * height * bpp;
    uint* p = (uint*)graphic_video_mem;
    while (n_bytes > 0) {
        *p++ = 0;
        n_bytes -= 4;
    }
}

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

void draw_bmp_at(char* fpath, int x, int y) {
    FILE* fp = fopen(fpath);
    size_t size = fsize(fpath);
    char* bmp = malloc(size);
    fread(fp, size, bmp);

    BitmapHeader* bh = (BitmapHeader*)bmp;
    BitmapInfoHeader* bih = (BitmapInfoHeader*)(bmp + sizeof(BitmapHeader));
    uchar* pixels = (uchar*)(bmp + bh->offset);
    int width = bih->width;
    int height = bih->height;
    int bytes_per_row = width * bytes_per_pixel;
    if (bytes_per_row % 4 != 0) {
        bytes_per_row += 4 - bytes_per_row % 4;
    }
    int bpp = get_bytes_per_pixel();
    int pitch = get_pitch();

    int screen_width = get_screen_width();
    int screen_height = get_screen_height();
    int left = x;
    int top = y;
    int right = min(left + width, screen_width);
    int bottom = min(top + height, screen_height);

    ushort bytes_per_pixel = bih->bpp / 8;
    uint offset_data = (height - 1) * bytes_per_row;
    uint bytes_per_row_mem = screen_width * bpp;
    uint offset_mem = top * pitch + left * bpp;
    uint bytes_copy_row = (right - left) * bytes_per_pixel;
    for (int i = top; i < bottom; ++i) {
        memcpy(graphic_video_mem + offset_mem, pixels + offset_data, bytes_copy_row);
        offset_data -= bytes_per_row;
        offset_mem += bytes_per_row_mem;
    }
    free(bmp);
}

void draw_bmp(char* fpath) {
    FILE* fp = fopen(fpath);
    char* bmp = malloc(512);
    fread(fp, 512, bmp);
    BitmapHeader* bh = (BitmapHeader*)bmp;
    BitmapInfoHeader* bih = (BitmapInfoHeader*)(bmp + sizeof(BitmapHeader));
    int width = bih->width;
    int height = bih->height;
    int screen_width = get_screen_width();
    int screen_height = get_screen_height();
    int left = (screen_width - width) / 2;
    int top = (screen_height - height) / 2;
    draw_bmp_at(fpath, left, top);
}
