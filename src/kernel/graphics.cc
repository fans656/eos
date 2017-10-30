#include "graphics.h"
#include "filesystem.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "util.h"
#include "asm.h"
#include "mouse.h"

// http://www.delorie.com/djgpp/doc/rbinter/ix/10/4F.html

struct  __attribute__ ((packed)) vbe_mode_info_structure {
   ushort attributes;
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
};

#define CHARSET_SIZE 128

vbe_mode_info_structure* mode_info = (vbe_mode_info_structure*)(0x600 + KERNEL_BASE);
uchar* graphic_video_mem;

int screen_width;
int screen_height;
int screen_pitch;
int screen_bpp;
int screen_bytes;

int font_bmp_width;
int font_bmp_height;
uint font_glyph_width;
uint font_glyph_height;
uint font_bpr;  // bytes per row of the font bmp image

uint* font_data;

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

void blit(
        const uchar* buffer, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    int dst_pitch = screen_pitch;
    int dst_offset = dst_left << 2;
    int src_offset = src_left << 2;
    uchar* dst = (uchar*)(graphic_video_mem + dst_top * dst_pitch + dst_offset);
    const uchar* src = (const uchar*)(buffer + src_top * src_pitch + src_offset);
    while (height--) {
        asm volatile(
                "mov esi, %0;"
                "mov edi, %1;"
                "mov ecx, %2;"
                "cld;"
                "rep movsd;"
                :: "a"(src), "b"(dst), "c"(width)
                );
        dst += dst_pitch;
        src += src_pitch;
    }
}

void init_graphics() {
    graphic_video_mem = (uchar*)mode_info->framebuffer;
    if (!graphic_video_mem) {
        return;
    }
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
    reload_cr3(kernel_pgdir);
    
    // init font
    uchar* font_bmp = (uchar*)load_file("/font/font.bmp");
    uchar* font_bmp_data = (uchar*)bmp_data(font_bmp);

    BitmapInfoHeader* bih = (BitmapInfoHeader*)(font_bmp + sizeof(BitmapHeader));
    font_glyph_width = bmp_width(font_bmp) / CHARSET_SIZE;
    font_glyph_height = bmp_height(font_bmp);
    font_bpr = bmp_pitch(font_bmp);

    font_bmp_width = bmp_width(font_bmp);
    font_bmp_height = bmp_height(font_bmp);
    size_t font_data_size = font_bmp_width * font_bmp_height;
    //font_data = new uint[font_data_size];
    font_data = (uint*)named_malloc(font_data_size * sizeof(uint), "font_data");
    uint* q = font_data;
    for (int y = 0; y < font_bmp_height; ++y) {
        for (int x = 0; x < font_bmp_width; ++x) {
            uchar* p = font_bmp_data + (font_glyph_height - 1 - y) * font_bpr + x * 3;
            uint color = *p | (*(p + 1) << 8) | (*(p + 2) << 16);
            *q++ = color;
        }
    }
    delete font_bmp;
    
    // init virtual console
    COLS = screen_width / font_glyph_width;
    ROWS = screen_height / font_glyph_height;
    int video_mem_size = COLS * ROWS * 2;
    console_video_mem = (ushort*)named_malloc(video_mem_size, "video_mem");
    memset(console_video_mem, 0, video_mem_size);
    cur_row = cur_col = 0;
    
    // init mouse
    init_mouse(screen_width, screen_height);
}

int get_screen_width() {
    return screen_width;
}

int get_screen_height() {
    return screen_height;
}

int get_screen_pitch() {
    return screen_pitch;
}

int get_screen_bpp() {
    return screen_bpp;
}

void draw_pixel(int x, int y, uint color) {
    uint i = y * screen_pitch + x * screen_bpp;
    graphic_video_mem[i] = color & 0xff;
    graphic_video_mem[i + 1] = (color >> 8) & 0xff;
    graphic_video_mem[i + 2] = (color >> 16) & 0xff;
}

void invert_pixel(int x, int y) {
    uint i = y * screen_pitch + x * screen_bpp;
    graphic_video_mem[i] = ~graphic_video_mem[i];
    graphic_video_mem[i + 1] = ~graphic_video_mem[i + 1];
    graphic_video_mem[i + 2] = ~graphic_video_mem[i + 2];
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

void draw_char(char ch, int row, int col) {
    int src_left = ch * font_glyph_width;
    int dst_left = col * font_glyph_width;
    int dst_top = row * font_glyph_height;
    blit((uchar*)font_data, font_bmp_width * screen_bpp,
            src_left, 0, dst_left, dst_top,
            font_glyph_width, font_glyph_height);
}

void sync_console_at(int row, int col) {
    if (font_data) {
        ushort val = CHAR(row, col);
        if (val) {
            draw_char((char)(val & 0xff), row, col);
        }
    }
}

void sync_console() {
    if (font_data) {
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

void get_screen_info(ScreenInfo* info) {
    info->width = screen_width;
    info->height = screen_height;
    info->bpp = screen_bpp;
    info->pitch = screen_pitch;
    info->video_mem = graphic_video_mem;
    info->font_data = (uchar*)font_data;
    info->font_pitch = font_bmp_width * 4;
    info->font_glyph_width = font_glyph_width;
    info->font_glyph_height = font_glyph_height;
}
