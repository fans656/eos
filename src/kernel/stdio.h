#include "def.h"

extern int ROWS, COLS;
extern ushort* video_mem;

void init_console();

void set_cursor(int row, int col);
void disable_cursor();
void enable_cursor();

void putchar(char ch);

void _printf(char** pfmt);
void printf(char* fmt, ...);
void panic(char* fmt, ...);

void print_hex_1(uint val);
