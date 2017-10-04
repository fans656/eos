#include "def.h"
#include "asm.h"
#include "graphics.h"

int ROWS = 25, COLS = 80;
ushort* video_mem = (ushort*)P2V(VIDEO_MEM);

#define GRAY(ch) (0x0700 | (ch))

int cur_row = 0, cur_col = 0;

// http://wiki.osdev.org/Text_Mode_Cursor
void set_cursor(int row, int col) {
    cur_row = row;
    cur_col = col;
    ushort pos = row * COLS + col;
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
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            if (row < ROWS - 1) {
                CHAR(row, col) = CHAR(row + 1, col);
            } else {
                CHAR(row, col) = GRAY(' ');
            }
        }
    }
    sync_console();
}

static inline void carrige() {
    cur_col = 0;
    set_cursor(cur_row, cur_col);
}

void newline() {
    if (++cur_row == ROWS) {
        scroll_down();
        --cur_row;
    }
    carrige();
}

void newline2() {
    ++cur_row;
}

void putchar(char ch) {
    switch (ch) {
        case '\n':
            newline();
            return;
        case '\r':
            carrige();
            return;
    }
    CHAR(cur_row, cur_col) = GRAY(ch);
    sync_console_at(cur_row, cur_col);
    ++cur_col;
    if (cur_col == COLS) {
        newline();
    }
    set_cursor(cur_row, cur_col);
}

#define HEX(v) ((v) < 10 ? ((v) + '0') : ((v) - 10 + 'A'))

void print_hex_byte(uint val) {
    putchar(HEX(val >> 4));
    putchar(HEX(val & 0x0f));
}

void print_hex_dword(uint val) {
    print_hex_byte((val >> 24) & 0xff);
    print_hex_byte((val >> 16) & 0xff);
    print_hex_byte((val >> 8) & 0xff);
    print_hex_byte(val & 0xff);
}

int print_int(int val, int width) {
    int res = 0;
    bool negative = false;
    uint cnt = 0;
    if (val < 0) {
        negative = true;
        val = -val;
        ++cnt;
    }
    uint base = 1;
    while (base * 10 <= val) {
        base *= 10;
        ++cnt;
    }
    if (width > cnt) {
        for (int i = 0; i < width - cnt; ++i) {
            putchar(' ');
            ++res;
        }
    }
    if (negative) {
        putchar('-');
        ++res;
    }
    while (base) {
        putchar(val / base + '0');
        val %= base;
        base /= 10;
        ++res;
    }
    return res;
}

int print_str(char* s, int width) {
    int res = 0;
    while (*s) {
        putchar(*s++);
        --width;
        ++res;
    }
    while (width-- > 0) {
        putchar(' ');
        ++res;
    }
    return res;
}

bool isdigit(char ch) {
    return '0' <= ch && ch <= '9';
}

int printf_get_width(const char** pp) {
    int res = 0;
    while (isdigit(**pp)) {
        res = res * 10 + (**pp - '0');
        ++*pp;
    }
    return res;
}

int _printf(const char** pfmt) {
    int res = 0;
    uint* arg = (uint*)pfmt + 1;
    int width = 0;
    for (const char* p = *pfmt; *p; ++p) {
        switch (*p) {
            case '%':
                ++p;
                width = printf_get_width(&(p));
                switch (*p) {
                    case 'x':
                        print_hex_dword(*arg++);
                        res += 8;
                        break;
                    case 'd':
                        res += print_int(*(int*)arg++, width);
                        break;
                    case 's':
                        res += print_str((char*)*arg++, width);
                        break;
                }
                break;
            default:
                putchar(*p);
                ++res;
                break;
        }
    }
    return res;
}

int printf(const char* fmt, ...) {
    return _printf(&fmt);
}

void init_console() {
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            CHAR(row, col) = GRAY(' ');
        }
    }
    enable_cursor();
    set_cursor(cur_row, cur_col);
}

void panic(const char* fmt, ...) {
    _printf(&fmt);
    while (true) {
        asm("hlt");
    }
}
