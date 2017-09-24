#include "types.h"
#include "conf.h"
#include "asm.h"

#define CHAR(row, col) (*(VIDEO_MEM + (row) * 80 + (col)))

#define GRAY(ch) (0x0700 | (ch))

int cur_row = 0, cur_col = 0;

// http://wiki.osdev.org/Text_Mode_Cursor
void set_cursor(int row, int col) {
    cur_row = row;
    cur_col = col;
    ushort pos = row * 80 + col;
    outb(0x3d4, 0x0f);
    outb(0x3d5, pos & 0xff);
    outb(0x3d4, 0x0e);
    outb(0x3d5, pos >> 8);
}

void disable_cursor() {
    outb(0x3d4, 0x0a);
    outb(0x3d5, 0x3f);
}

void enable_cursor() {
    outb(0x3d4, 0x0a);
    outb(0x3d5, inb(0x3d5) & 0xc0 | 0);
    outb(0x3d4, 0x0b);
    outb(0x3d5, inb(0x3e0) & 0xe0 | 15);
}

void scroll_down() {
    for (int row = 0; row < 25; ++row) {
        for (int col = 0; col < 80; ++col) {
            if (row < 24) {
                CHAR(row, col) = CHAR(row + 1, col);
            } else {
                CHAR(row, col) = GRAY(' ');
            }
        }
    }
}

void newline() {
    if (++cur_row == 25) {
        scroll_down();
        --cur_row;
    }
    cur_col = 0;
    set_cursor(cur_row, cur_col);
}

void putchar(char ch) {
    switch (ch) {
        case '\n':
            newline();
            return;
    }
    CHAR(cur_row, cur_col++) = GRAY(ch);
    if (cur_col == 80) {
        newline();
    }
    set_cursor(cur_row, cur_col);
}

#define HEX(v) ((v) < 10 ? ((v) + '0') : ((v) - 10 + 'A'))

void print_hex_1(uint val) {
    putchar(HEX(val >> 4));
    putchar(HEX(val & 0x0f));
}

void print_hex_4(uint val) {
    print_hex_1((val >> 24) & 0xff);
    print_hex_1((val >> 16) & 0xff);
    print_hex_1((val >> 8) & 0xff);
    print_hex_1(val & 0xff);
}

void print_int(int val) {
    if (val < 0) {
        putchar('-');
    }
    uint base = 1;
    while (base * 10 < val) {
        base *= 10;
    }
    while (base) {
        putchar(val / base + '0');
        val %= base;
        base /= 10;
    }
}

void print_str(char* s) {
    while (*s) {
        putchar(*s++);
    }
}

void _printf(char** pfmt) {
    uint* arg = (uint*)pfmt + 1;
    for (char* p = *pfmt; *p; ++p) {
        switch (*p) {
            case '%':
                switch (*++p) {
                    case 'x':
                        print_hex_4(*arg++);
                        break;
                    case 'd':
                        print_int(*arg++);
                        break;
                    case 's':
                        print_str((char*)*arg++);
                        break;
                }
                break;
            default:
                putchar(*p);
                break;
        }
    }
}

void printf(char* fmt, ...) {
    _printf(&fmt);
}

void init_console() {
    for (int row = 0; row < 25; ++row) {
        for (int col = 0; col < 80; ++col) {
            CHAR(row, col) = GRAY(' ');
        }
    }
    enable_cursor();
    set_cursor(cur_row, cur_col);
}

void panic(char* fmt, ...) {
    _printf(&fmt);
    while (true) {
        asm("hlt");
    }
}
