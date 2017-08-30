#include "textmode.h"
#include "util.h"

uint16_t GRAY_FG = 0x0700;
uint16_t WHITE_FG = 0x0f00;

uint8_t N_ROWS = 25;
uint8_t N_COLS = 80;

uint8_t cursor_cur_row = 0;
uint8_t cursor_cur_col = 0;

uint16_t* VIDEO_MEM = (uint16_t*)0xb8000;
uint16_t* VGA_INDEX_BASE_PORT_ADDR = (uint16_t*)0x0463;

void scroll_down_one_line() {
    for (int i = 0; i < N_ROWS - 1; ++i) {
        for (int j = 0; j < N_COLS; ++j) {
            VIDEO_MEM[i * N_COLS + j] = VIDEO_MEM[(i + 1) * N_COLS + j];
        }
    }
    for (int j = 0; j < N_COLS; ++j) {
        VIDEO_MEM[(N_ROWS - 1) * N_COLS + j] = GRAY_FG | ' ';
    }
}

void put_newline() {
    if (cursor_cur_row == N_ROWS - 1) {
        scroll_down_one_line();
    } else {
        ++cursor_cur_row;
    }
    cursor_cur_col = 0;
}

void put_char(char ch) {
    if (ch == '\n') {
        put_newline();
    } else {
        VIDEO_MEM[cursor_cur_row * N_COLS + cursor_cur_col] = GRAY_FG | ch;
        if (++cursor_cur_col == N_COLS) {
            put_newline();
        }
    }
    set_cursor_pos(cursor_cur_row, cursor_cur_col);
}

void print_str(char* s) {
    while (*s) {
        put_char(*s++);
    }
}

char four_bits_to_char(uint8_t bits) {
    return bits < 10 ? bits + '0' : bits - 10 + 'A';
}

void print_byte(uint8_t val) {
    put_char(four_bits_to_char(val >> 4));
    put_char(four_bits_to_char(val & 0x0f));
}

void print_mem(void* addr, int n_bytes) {
    uint8_t* p = addr;
    for (int i = 0; i < n_bytes; ++i) {
        if (i) {
            put_char(' ');
        }
        print_byte(*p++);
    }
}

void print_int(int val) {
    if (val < 0) {
        put_char('-');
        val = -val;
    }
    int base = 1;
    while (base * 10 < val) {
        base *= 10;
    }
    while (base) {
        char ch = '0' + val / base;
        val %= base;
        base /= 10;
        put_char(ch);
    }
}

void clear_screen() {
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 80; ++j) {
            VIDEO_MEM[i * 80 + j] = GRAY_FG | ' ';
        }
    }
}

void set_cursor_pos(uint8_t row, uint8_t col) {
    uint16_t port = *VGA_INDEX_BASE_PORT_ADDR;
    uint16_t offset = row * N_ROWS + col;
    outb(port, 0x0f);
    outb(port + 1, offset & 0xff);
    outb(port, 0x0e);
    outb(port + 1, (offset >> 8) & 0xff);
}
