#include <stdint.h>
#include "util.h"

extern uint16_t GRAY_FG;
extern uint16_t WHITE_FG;
extern uint16_t* VIDEO_MEM;

void clear_screen();
void put_char(char ch);
void print_byte(uint8_t val);
void print_int(int val);
void print_str(char* s);
void print_mem(void* addr, int n_bytes);
void set_cursor_pos(uint8_t row, uint8_t col);
