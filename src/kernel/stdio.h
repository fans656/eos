#include "def.h"

extern int ROWS, COLS;
extern int cur_row, cur_col;
extern ushort* video_mem;

void init_console();

void set_cursor(int row, int col);
void disable_cursor();
void enable_cursor();

void putchar(char ch);

int _printf(const char** pfmt);
int printf(const char* fmt, ...);
void panic(const char* fmt, ...);

void print_hex_1(uint val);
