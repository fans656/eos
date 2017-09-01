#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>
#include "util.h"

#define KEY_UP (0x48 | 0x80)
#define KEY_LEFT (0x4b | 0x80)
#define KEY_RIGHT (0x4d | 0x80)
#define KEY_DOWN (0x50 | 0x80)

extern uint16_t GRAY_FG;
extern uint16_t WHITE_FG;
extern uint16_t* VIDEO_MEM;
extern uint8_t SCANCODE_TO_KEY[];

extern uint8_t cursor_cur_row;
extern uint8_t cursor_cur_col;

extern int current_key;
extern int num_key_pressed;
extern bool key_states[];

void clear_screen();
void put_char(char ch);
void print_byte(uint8_t val);
void print_word(uint16_t val);
void print_int(int val);
void print_str(char* s);
void print_mem(void* addr, int n_bytes);
void set_cursor_row_col(uint8_t row, uint8_t col);
int get_char();
void update_key_states(uint8_t scancode);
int get_char_nonblocking();
void printf(char* fmt, ...);

#endif
