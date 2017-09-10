#include "draw.h"
#include "filesystem.h"
#include "malloc.h"
#include "io.h"

// http://www.delorie.com/djgpp/doc/rbinter/ix/10/4F.html

typedef struct  __attribute__ ((packed)) {
   uint16_t attributes;  // deprecated
   uint8_t window_a;    // deprecated
   uint8_t window_b;   // deprecated
   uint16_t granularity;  // deprecated
   uint16_t window_size;
   uint16_t segment_a;
   uint16_t segment_b;
   uint32_t win_func_ptr;  // deprecated
   uint16_t pitch;      
   uint16_t width;     
   uint16_t height;   
   uint8_t w_char;   
   uint8_t y_char;  
   uint8_t planes;
   uint8_t bpp;    
   uint8_t banks; 
   uint8_t memory_model;
   uint8_t bank_size;
   uint8_t image_pages;
   uint8_t reserved0;

   uint8_t red_mask;
   uint8_t red_position;
   uint8_t green_mask;
   uint8_t green_position;
   uint8_t blue_mask;
   uint8_t blue_position;
   uint8_t reserved_mask;
   uint8_t reserved_position;
   uint8_t direct_color_attributes;

   uint32_t framebuffer;      
   uint32_t off_screen_mem_off;
   uint16_t off_screen_mem_size;
   uint8_t reserved1[206];
} vbe_mode_info_structure;

static vbe_mode_info_structure* mode_info = (vbe_mode_info_structure*)0x500;
static uint8_t* graphic_video_mem;
static int screen_width;
static int screen_height;
static int pitch;
static int bytes_per_pixel;

void graphic_init() {
    graphic_video_mem = (uint8_t*)mode_info->framebuffer;
    screen_width = mode_info->width;
    screen_height = mode_info->height;
    pitch = mode_info->pitch;
    bytes_per_pixel = mode_info->bpp / 8;
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

void draw_pixel(int x, int y, uint32_t color) {
    uint32_t i = y * pitch + x * bytes_per_pixel;
    graphic_video_mem[i] = color & 0xff;
    graphic_video_mem[i + 1] = (color >> 8) & 0xff;
    graphic_video_mem[i + 2] = (color >> 16) & 0xff;
}

void fill_rect(int left, int top, int width, int height, uint32_t color) {
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
    uint32_t n_bytes = width * height * bpp;
    uint32_t* p = (uint32_t*)graphic_video_mem;
    while (n_bytes > 0) {
        *p++ = 0;
        n_bytes -= 4;
    }
}

typedef struct __attribute__((packed)) {
    char signature[2];
    uint32_t file_size;
    uint32_t _reserved;
    uint32_t offset;
} BitmapHeader;

typedef struct __attribute__((packed)) {
    uint32_t header_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint8_t _notcare[20];
} BitmapInfoHeader;

void draw_bmp(char* fpath) {
    FILE* fp = fopen(fpath);
    uint64_t size = fp->entry->size;
    char* bmp = malloc(size);
    fread(fp, size, bmp);

    BitmapHeader* bh = (BitmapHeader*)bmp;
    BitmapInfoHeader* bih = (BitmapInfoHeader*)(bmp + sizeof(BitmapHeader));
    uint8_t* pixels = (uint8_t*)(bmp + bh->offset);
    int width = bih->width;
    int height = bih->height;
    int bytes_per_row = width * bytes_per_pixel;
    if (bytes_per_row % 4 != 0) {
        bytes_per_row += 4 - bytes_per_row % 4;
    }
    int bpp = get_bytes_per_pixel();

    int screen_width = get_screen_width();
    int screen_height = get_screen_height();
    int left = (screen_width - width) / 2;
    int top = (screen_height - height) / 2;

    uint16_t bytes_per_pixel = bih->bpp / 8;
    uint32_t offset_data = (height - 1) * bytes_per_row;
    uint32_t bytes_per_row_mem = screen_width * bpp;
    uint32_t offset_mem = top * bytes_per_row_mem + left * bpp;
    for (int i = 0; i < height; ++i) {
        memcpy(pixels + offset_data, graphic_video_mem + offset_mem, bytes_per_row);
        offset_data -= bytes_per_row;
        offset_mem += bytes_per_row_mem;
    }
}
