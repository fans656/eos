#ifndef _KERNEL_STDIO_H
#define _KERNEL_STDIO_H

#include "def.h"
#include "filesystem.h"

typedef void (*putchar_func_t)(char ch);

extern int ROWS, COLS;
extern int cur_row, cur_col;
extern ushort* console_video_mem;
extern putchar_func_t putchar;

void init_console();

void set_cursor(int row, int col);
void disable_cursor();
void enable_cursor();

//void putchar(char ch);

int _printf(const char** pfmt);
int _debug(const char** pfmt);
int printf(const char* fmt, ...);
int debug(const char* fmt, ...);
void panic(const char* fmt, ...);

void debug_on();
void debug_off();

void print_hex_byte(uint val);

#endif
