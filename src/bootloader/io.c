/*
http://www.computer-engineering.org/ps2keyboard/scancodes1.html
 */
#include <stdint.h>
#include <stdbool.h>
#include "io.h"
#include "util.h"

uint16_t GRAY_FG = 0x0700;
uint16_t WHITE_FG = 0x0f00;

uint8_t N_ROWS = 25;
uint8_t N_COLS = 80;

uint8_t cursor_cur_row = 0;
uint8_t cursor_cur_col = 0;

bool inputting;

uint8_t input_beg_row;
uint8_t input_beg_col;

uint8_t input_end_row;
uint8_t input_end_col;

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

void setchar(int ch) {
    VIDEO_MEM[cursor_cur_row * N_COLS + cursor_cur_col] = GRAY_FG | ch;
}

void putchar(int ch) {
    if (ch == '\n') {
        put_newline();
    } else {
        setchar(ch);
        if (++cursor_cur_col == N_COLS) {
            put_newline();
        }
    }
    set_cursor_row_col(cursor_cur_row, cursor_cur_col);
    if (inputting) {
        input_end_row = cursor_cur_row;
        input_end_col = cursor_cur_col;
    }
}

void print_str(char* s) {
    while (*s) {
        putchar(*s++);
    }
}

void print_strn(char* s, int n) {
    while (n--) {
        putchar(*s++);
    }
}

char four_bits_to_char(uint8_t bits) {
    return bits < 10 ? bits + '0' : bits - 10 + 'A';
}

void print_byte(uint8_t val) {
    putchar(four_bits_to_char(val >> 4));
    putchar(four_bits_to_char(val & 0x0f));
}

void print_word(uint16_t val) {
    print_byte(val & 0xff);
    putchar(' ');
    print_byte(val >> 8);
}

void print_mem(void* addr, int n_bytes) {
    uint8_t* p = addr;
    for (int i = 0; i < n_bytes; ++i) {
        if (i % 16 == 0) {
            printf("%p  ", i);
        }
        if (i % 16) {
            putchar(' ');
        }
        print_byte(*p++);
        if (i % 8 == 7) {
            putchar(' ');
        }
        if (i % 16 == 15) {
            putchar('\n');
        }
    }
}

void print_int(int val) {
    if (val < 0) {
        putchar('-');
        val = -val;
    }
    int base = 1;
    while (base * 10 <= val) {
        base *= 10;
    }
    while (base) {
        char ch = '0' + val / base;
        val %= base;
        base /= 10;
        putchar(ch);
    }
}

void print_64(uint64_t val) {
    if (val < 0) {
        putchar('-');
        val = -val;
    }
    uint64_t base = 1;
    while (base * 10 <= val) {
        base *= 10;
    }
    while (base) {
        char ch = '0' + val / base;
        val %= base;
        base /= 10;
        putchar(ch);
    }
}

void clear_screen() {
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 80; ++j) {
            VIDEO_MEM[i * 80 + j] = GRAY_FG | ' ';
        }
    }
    set_cursor_row_col(0, 0);
}

void set_cursor_row_col(uint8_t row, uint8_t col) {
    cursor_cur_row = row;
    cursor_cur_col = col;
    uint16_t port = *VGA_INDEX_BASE_PORT_ADDR;
    uint16_t offset = row * N_COLS + col;
    outb(port, 0x0f);
    outb(port + 1, offset & 0xff);
    outb(port, 0x0e);
    outb(port + 1, (offset >> 8) & 0xff);
}

void sys_printf(void* p_arg0) {
    char* fmt = *(char**)p_arg0;
    char* arg = (char*)(p_arg0 + 4);
    for (char* p = fmt; *p; ++p) {
        if (*p != '%') {
            putchar(*p);
        } else {
            ++p;
            int width = 8;
            if ('0' <= *p && *p <= '9') {
                switch (*(p + 1)) {
                    case '2':
                        width = 2;
                        break;
                    case '4':
                        width = 4;
                        break;
                }
                p += 2;
            }
            if (*p == 'l' && *(p + 1) == 'l' && *(p + 2) == 'd') {
                p += 2;
                print_64(*(uint64_t*)arg);
                arg += 8;
                continue;
            }
            switch (*p) {
                case 'c':
                    putchar(*(char*)arg);
                    arg += 4;
                    break;
                case 'd':
                    print_int(*(int*)arg);
                    arg += 4;
                    break;
                case 's':
                    print_str((char*)(*(char**)arg));
                    arg += 4;
                    break;
                case 'x':
                    if (width >= 8) {
                        print_byte(*((char*)arg + 3));
                        print_byte(*((char*)arg + 2));
                    }
                    if (width >= 4) {
                        print_byte(*((char*)arg + 1));
                    }
                    if (width >= 2) {
                        print_byte(*((char*)arg));
                    }
                    arg += 4;
                    break;
                case 'p':
                    print_str("0x");
                    print_byte(*((char*)arg + 3));
                    print_byte(*((char*)arg + 2));
                    print_byte(*((char*)arg + 1));
                    print_byte(*((char*)arg));
                    arg += 4;
                    break;
                case 'h':
                    print_byte(*((char*)arg));
                    arg += 4;
                    break;
                case '%':
                    putchar('%');
                    break;
                default:
                    printf("%%%c", *p);
                    break;
            }
        }
    }
}

/*
 * printf("%d", 656);
 * printf("%s", "hello");
 * printf("%p", &x);
 * printf("%c", 'a');
 *
 * printf("%02x", 0x12);  // => 12
 * printf("%04x", 0x12);  // => 0012
 * printf("%08x", 0x12);  // => 00000012
 * printf("%x", 0x12345678);  // => 12345678
 */
void printf(char* fmt, ...) {
    sys_printf(&fmt);
}
