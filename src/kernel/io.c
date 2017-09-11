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

uint16_t* VIDEO_MEM = (uint16_t*)0xb8000;
uint16_t* VGA_INDEX_BASE_PORT_ADDR = (uint16_t*)0x0463;

uint8_t SCANCODE_TO_KEY[128] = {
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P',   0,   0,   0,   0, 'A', 'S',

    'D', 'F', 'G', 'H', 'J', 'K', 'L',   0,
      0,   0,   0,   0, 'Z', 'X', 'C', 'V',
    'B', 'N', 'M',   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      KEY_UP,   0,   0,   KEY_LEFT,   0,   KEY_RIGHT,   0,   0,
      KEY_DOWN,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,

      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
};

int current_key = -1;
int num_key_pressed = 0;
bool key_states[128];

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
    set_cursor_row_col(cursor_cur_row, cursor_cur_col);
}

void print_str(char* s) {
    while (*s) {
        put_char(*s++);
    }
}

void print_strn(char* s, int n) {
    while (n--) {
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

void print_word(uint16_t val) {
    print_byte(val & 0xff);
    put_char(' ');
    print_byte(val >> 8);
}

void print_mem(void* addr, int n_bytes) {
    uint8_t* p = addr;
    for (int i = 0; i < n_bytes; ++i) {
        if (i % 16 == 0) {
            printf("%p  ", i);
        }
        if (i % 16) {
            put_char(' ');
        }
        print_byte(*p++);
        if (i % 8 == 7) {
            put_char(' ');
        }
        if (i % 16 == 15) {
            put_char('\n');
        }
    }
}

void print_int(int val) {
    if (val < 0) {
        put_char('-');
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
        put_char(ch);
    }
}

void print_64(uint64_t val) {
    if (val < 0) {
        put_char('-');
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
        put_char(ch);
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

int get_char() {
    while (1) {
        int ch = get_char_nonblocking();
        if (ch != -1) {
            return ch;
        } else {
            asm("hlt");
        }
    }
}

int get_char_nonblocking() {
    return current_key;
}

void update_key_states(uint8_t scancode) {
    uint8_t key = SCANCODE_TO_KEY[scancode & 0x7f];
    bool up = scancode & 0x80;
    if (up) {
        if (--num_key_pressed == 0) {
            current_key = -1;
        }
        key_states[key] = 0;
    } else {
        if (!key_states[key]) {
            ++num_key_pressed;
        }
        key_states[key] = 1;
        current_key = key;
    }
}

void sys_printf(void* p_arg0) {
    char* fmt = *(char**)p_arg0;
    char* arg = (char*)(p_arg0 + 4);
    for (char* p = fmt; *p; ++p) {
        if (*p != '%') {
            put_char(*p);
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
                    put_char(*(char*)arg);
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
                    put_char('%');
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
