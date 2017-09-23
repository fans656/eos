#include "types.h"

#define VIDEO_MEM 0xc00b8000

int cur_row = 0, cur_col = 0;

void newline() {
    ++cur_row;
    cur_col = 0;
}

void putchar(char ch) {
    switch (ch) {
        case '\n':
            newline();
            return;
    }
    ushort* mem = (ushort*)VIDEO_MEM + cur_row * 80 + cur_col++;
    *mem = 0x0700 | ch;
    if (cur_col == 80) {
        ++cur_row;
        cur_col = 0;
    }
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

void printf(char* fmt, ...) {
    uint* arg = (uint*)&fmt + 1;
    for (char* p = fmt; *p; ++p) {
        switch (*p) {
            case '%':
                switch (*++p) {
                    case 'x':
                        print_hex_4(*arg);
                        break;
                }
                break;
            default:
                putchar(*p);
                break;
        }
    }
}

void clear_console() {
    for (int i = 0; i < 25; ++i) {
        for (int j = 0; j < 80; ++j) {
            *((ushort*)VIDEO_MEM + i * 80 + j) = 0x0700 | ' ';
        }
    }
}
