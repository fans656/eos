#include "graphics.h"
#include "filesystem.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "util.h"

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

#define CHARSET_SIZE 128

vbe_mode_info_structure* mode_info = (vbe_mode_info_structure*)(0x600 + KERNEL_BASE);
uchar* graphic_video_mem;
int screen_width;
int screen_height;
int screen_pitch;
int screen_bpp;

uchar* font_bmp;
uchar* font_bmp_data;
uint font_glyph_width;
uint font_glyph_height;
uint font_bpr;  // bytes per row of the font bmp image

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

int bmp_pitch(void* bmp) {
    BitmapInfoHeader* bih = BMP_INFO_HEADER(bmp);
    return align4(bih->width * bih->bpp / 8);
}

void bmp_blit_nocheck(void* bmp,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    int dst_pitch = screen_pitch;
    int src_pitch = bmp_pitch(bmp);
    int dst_offset = dst_left * screen_bpp;
    int src_offset = src_left * screen_bpp;
    int n = width * screen_bpp;
    uchar* dst = graphic_video_mem + dst_top * dst_pitch + dst_offset;
    uchar* src = bmp_data(bmp) + (bmp_height(bmp) - 1 - src_top) * src_pitch + src_offset;
    while (height--) {
        memcpy(dst, src, n);
        dst += dst_pitch;
        src -= src_pitch;
    }
}

void bmp_blit(void* bmp,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {

    int src_width = bmp_width(bmp);
    int src_height = bmp_height(bmp);
    
    dst_left = restricted(dst_left, 0, screen_width);
    dst_top = restricted(dst_top, 0, screen_height);

    src_left = restricted(src_left, 0, src_width);
    src_top = restricted(src_top, 0, src_height);
    
    width = min(min(width, screen_width - dst_left), src_width - src_left);
    height = min(min(height, screen_height - dst_top), src_height - src_top);
    
    if (width && height) {
        bmp_blit_nocheck(bmp, src_left, src_top, dst_left, dst_top, width, height);
    }
}

void bmp_draw_at(void* bmp, int left, int top) {
    bmp_blit(bmp, 0, 0, left, top, bmp_width(bmp), bmp_height(bmp));
}

void init_graphics() {
    graphic_video_mem = (uchar*)mode_info->framebuffer;
    screen_width = mode_info->width;
    screen_height = mode_info->height;
    screen_pitch = mode_info->pitch;
    screen_bpp = mode_info->bpp / 8;

    // map graphic memory
    uint beg = (uint)graphic_video_mem;
    for (size_t sz = 0; sz < 16 * MB; sz += 4 * MB) {
        kernel_pgdir[beg >> 22] = beg | PTE_P | PTE_W | PTE_PS;
        beg += 4 * MB;
    }
    
    // init font
    font_bmp = load_file("/font/font.bmp");
    font_bmp_data = bmp_data(font_bmp);

    BitmapInfoHeader* bih = (BitmapInfoHeader*)(font_bmp + sizeof(BitmapHeader));
    font_glyph_width = bmp_width(font_bmp) / CHARSET_SIZE;
    font_glyph_height = bmp_height(font_bmp);
    font_bpr = bmp_pitch(font_bmp);
    
    // init virtual console
    COLS = screen_width / font_glyph_width;
    ROWS = screen_height / font_glyph_height;

    int video_mem_size = COLS * ROWS * 2;
    video_mem = named_malloc(video_mem_size, "video_mem");
    memset(video_mem, 0, video_mem_size);
    cur_row = cur_col = 0;
}

int get_screen_width() {
    return screen_width;
}

int get_screen_pitch() {
    return screen_pitch;
}

int get_screen_bpp() {
    return screen_bpp;
}

int get_screen_height() {
    return screen_height;
}

void draw_pixel(int x, int y, uint color) {
    uint i = y * screen_pitch + x * screen_bpp;
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
    int bpp = get_screen_bpp();
    int width = get_screen_width();
    int height = get_screen_height();
    uint n_bytes = width * height * bpp;
    uint* p = (uint*)graphic_video_mem;
    while (n_bytes > 0) {
        *p++ = 0;
        n_bytes -= 4;
    }
}

void draw_bmp_at(const char* fpath, int x, int y) {
    FILE* fp = fopen(fpath);
    size_t size = fsize(fp);
    char* bmp = named_malloc(size, fpath);
    fread(fp, size, bmp);

    BitmapHeader* bh = (BitmapHeader*)bmp;
    BitmapInfoHeader* bih = (BitmapInfoHeader*)(bmp + sizeof(BitmapHeader));
    uchar* pixels = (uchar*)(bmp + bh->offset);
    int width = bih->width;
    int height = bih->height;
    int bytes_per_row = width * screen_bpp;
    if (bytes_per_row % 4 != 0) {
        bytes_per_row += 4 - bytes_per_row % 4;
    }
    int bpp = get_screen_bpp();
    int screen_pitch = get_screen_pitch();

    int screen_width = get_screen_width();
    int screen_height = get_screen_height();
    int left = max(x, 0);
    int top = max(y, 0);
    int right = min(left + width, screen_width);
    int bottom = min(top + height, screen_height);

    ushort bytes_per_pixel = bih->bpp / 8;
    uint offset_data = (height - 1) * bytes_per_row;
    uint bytes_per_row_mem = screen_width * bpp;
    uint offset_mem = top * screen_pitch + left * bpp;
    uint bytes_copy_row = (right - left) * bytes_per_pixel;
    for (int i = max(top, 0); i < bottom; ++i) {
        memcpy(graphic_video_mem + offset_mem, pixels + offset_data, bytes_copy_row);
        offset_data -= bytes_per_row;
        offset_mem += bytes_per_row_mem;
    }
    //free(bmp);
}

void draw_bmp(char* fpath) {
    FILE* fp = fopen(fpath);
    char* bmp = named_malloc(512, "draw_bmp_512");
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

void draw_char(char ch, int row, int col) {
    int src_left = ch * font_glyph_width;
    int dst_left = col * font_glyph_width;
    int dst_top = row * font_glyph_height;
    bmp_blit_nocheck(font_bmp, src_left, 0, dst_left, dst_top,
            font_glyph_width, font_glyph_height);
}

void sync_console_at(int row, int col) {
    if (font_bmp) {
        ushort val = CHAR(row, col);
        if (val) {
            draw_char((char)(val & 0xff), row, col);
        }
    }
}

void sync_console() {
    if (font_bmp) {
        for (int i = 0; i < ROWS; ++i) {
            for (int j = 0; j < COLS; ++j) {
                ushort val = CHAR(i, j);
                if (val) {
                    draw_char((char)(val & 0xff), i, j);
                }
            }
        }
    }
}
