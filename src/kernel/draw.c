#include "draw.h"
#include "io.h"

// http://www.delorie.com/djgpp/doc/rbinter/ix/10/4F.html

typedef struct __attribute__((packed)) {
    char signature[4];
    uint16_t version;
    uint32_t oem_name;
    uint32_t flags;
    uint16_t* modes;
    uint16_t size;
    uint16_t oem_version;
    char* vendor_name;
    char* product_name;
    char* product_revision;
    uint16_t vbe_af_version;
    void* acc_modes;
    char _[216];
    char oem_data[256];
} SVGAInfo;

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
