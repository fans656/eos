#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdbool.h>
#include "util.h"

enum {
    KEY_ESC = 0x80,
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_TAB,
    KEY_LCTRL,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LALT,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
};

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
int getchar();
void putchar(int ch);
void print_byte(uint8_t val);
void print_word(uint16_t val);
void print_int(int val);
void print_str(char* s);
void print_strn(char* s, int n);
void print_mem(void* addr, int n_bytes);
void set_cursor_row_col(uint8_t row, uint8_t col);
void update_key_states(uint8_t scancode);
int async_getchar();
void sys_printf(void* p_arg0);
void printf(char* fmt, ...);

void begin_input();
void end_input();

#endif
