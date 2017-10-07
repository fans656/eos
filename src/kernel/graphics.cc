#include "graphics.h"
#include "filesystem.h"
#include "memory.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
#include "util.h"
#include "asm.h"

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
int screen_bytes;

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
    char* dst = (char*)(graphic_video_mem + dst_top * dst_pitch + dst_offset);
    char* src = (char*)(bmp_data(bmp) + (bmp_height(bmp) - 1 - src_top) * src_pitch + src_offset);
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

void memory_blit(
        const char* buffer, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height) {
    int dst_pitch = screen_pitch;
    int dst_offset = dst_left * screen_bpp;
    int src_offset = src_left * screen_bpp;
    int bytes_per_row = width * screen_bpp;
    char* dst = (char*)(graphic_video_mem + dst_top * dst_pitch + dst_offset);
    const char* src = (const char*)(buffer + src_top * src_pitch + src_offset);
    while (height--) {
        memcpy(dst, src, bytes_per_row);
        dst += dst_pitch;
        src += src_pitch;
    }
}

static inline void mouse_read_wait() {
    while (!(inb(0x64) & 1)) {
        continue;
    }
}

static inline void mouse_write_wait() {
    while (inb(0x64) & 2) {
        continue;
    }
}

static inline uchar mouse_command(uchar cmd) {
    mouse_write_wait();
    outb(0x64, 0xd4);  // command byte
    mouse_write_wait();
    outb(0x60, cmd);  // data byte
    mouse_read_wait();
    uchar ack = inb(0x60);  // ack from mouse
    return ack;
}

// http://forum.osdev.org/viewtopic.php?t=10247
// http://wiki.osdev.org/Mouse_Input
// http://wiki.osdev.org/PS/2_Mouse
void init_mouse() {
    uchar status;

    mouse_write_wait();  // enable aux mouse device
    outb(0x64, 0xa8);
    
    mouse_write_wait();  // get compaq status
    outb(0x64, 0x20);
    mouse_read_wait();
    status = inb(0x60);

    status |= 0x02;  // enable IRQ12
    status &= ~0x20;  // disable mouse clock
    
    mouse_write_wait();  // set compaq status
    outb(0x64, 0x60);
    mouse_write_wait();
    outb(0x60, status);

    mouse_command(0xf6);  // set defaults
    mouse_command(0xf4);  // enable data reporting
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
    font_bmp = (uchar*)load_file("/font/font.bmp");
    font_bmp_data = (uchar*)bmp_data(font_bmp);

    BitmapInfoHeader* bih = (BitmapInfoHeader*)(font_bmp + sizeof(BitmapHeader));
    font_glyph_width = bmp_width(font_bmp) / CHARSET_SIZE;
    font_glyph_height = bmp_height(font_bmp);
    font_bpr = bmp_pitch(font_bmp);
    
    // init virtual console
    COLS = screen_width / font_glyph_width;
    ROWS = screen_height / font_glyph_height;
    int video_mem_size = COLS * ROWS * 2;
    video_mem = (ushort*)named_malloc(video_mem_size, "video_mem");
    memset(video_mem, 0, video_mem_size);
    cur_row = cur_col = 0;
    
    // init mouse
    init_mouse();
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
