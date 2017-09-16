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

uint16_t* VIDEO_MEM = (uint16_t*)(0xb8000 + VADDR_BASE);
uint16_t* VGA_INDEX_BASE_PORT_ADDR = (uint16_t*)(0x0463 + VADDR_BASE);

uint8_t SCANCODE_TO_KEY[128] = {
    0, KEY_ESC,
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KEY_BACKSPACE,
    KEY_TAB, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KEY_ENTER,
    KEY_LCTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    KEY_LSHIFT,  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KEY_RSHIFT, 0,

    KEY_LALT,   ' ',   0,   0,   0,   0,   0,   0,

    0,   0,   0,   0,   0,   0,   0,   0,
    KEY_UP,   0,   0,   KEY_LEFT,   0,   KEY_RIGHT,   0,   0,
    KEY_DOWN,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,

    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
};

uint8_t NUM_SHIFTED[] = {'!', '@', '#', '$', '%', '^', '&', '*', '('};

int current_key = -1;
int num_key_pressed = 0;
bool key_states[128];

#define KEY_BUFFER_SIZE 4096
int key_buffer[KEY_BUFFER_SIZE];
int key_buffer_beg = 0;
int key_buffer_end = 0;

void key_buffer_push(int key) {
    int next_end = (key_buffer_end + 1) % KEY_BUFFER_SIZE;
    if (next_end == key_buffer_beg) {
        key_buffer_beg = (key_buffer_beg + 1) % KEY_BUFFER_SIZE;
    }
    key_buffer[key_buffer_end] = key;
    key_buffer_end = next_end;
}

void key_buffer_pop() {
    if (key_buffer_beg != key_buffer_end) {
        key_buffer_end = (key_buffer_end + KEY_BUFFER_SIZE - 1) % KEY_BUFFER_SIZE;
    }
}

int key_buffer_deque() {
    while (key_buffer_beg == key_buffer_end) {
        asm("hlt");
    }
    int res = key_buffer[key_buffer_beg];
    key_buffer_beg = (key_buffer_beg + 1) % KEY_BUFFER_SIZE;
    return res;
}

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

void cursor_back() {
    if (!(cursor_cur_row > input_beg_row || cursor_cur_col > input_beg_col)) {
        return;
    }
    if (cursor_cur_col == 0) {
        if (cursor_cur_row) {
            --cursor_cur_row;
            cursor_cur_col = N_COLS - 1;
        }
    } else {
        --cursor_cur_col;
    }
    set_cursor_row_col(cursor_cur_row, cursor_cur_col);
}

void cursor_forwrad() {
    if (!(cursor_cur_row < input_end_row || cursor_cur_col < input_end_col)) {
        return;
    }
    if (cursor_cur_col == N_COLS - 1) {
        if (cursor_cur_row < N_ROWS - 1) {
            ++cursor_cur_row;
            cursor_cur_col = 0;
        }
    } else {
        ++cursor_cur_col;
    }
    set_cursor_row_col(cursor_cur_row, cursor_cur_col);
}

void begin_input() {
    inputting = true;
    input_beg_row = cursor_cur_row;
    input_beg_col = cursor_cur_col;
}

void end_input() {
    inputting = false;
}

void put_backspace() {
    if (cursor_cur_row > input_beg_row
            || cursor_cur_col > input_beg_col) {
        cursor_back();
        setchar(' ');
    }
}

void putchar(int ch) {
    if (ch == '\n') {
        put_newline();
    } else if (ch == '\b') {
        put_backspace();
    } else {
        setchar(ch);
        if (++cursor_cur_col == N_COLS) {
            put_newline();
        }
    }
    set_cursor_row_col(cursor_cur_row, cursor_cur_col);
    if (inputting) {
        //input_end_row = cursor_cur_row;
        //input_end_col = cursor_cur_col;
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

int getchar() {
    return key_buffer_deque();
}

int async_getchar() {
    return current_key;
}

void on_key_down(int key) {
    if (0 < key && key < 128 || key == '`') {
        if (key_states[KEY_LSHIFT] || key_states[KEY_RSHIFT]) {
            if ('a' <= key && key <= 'z') {
                key &= ~0x20;
            } else if ('1' <= key && key <= '9') {
                key = NUM_SHIFTED[key - '1'];
            }
            switch (key) {
                case '0': key = ')'; break;
                case '-': key = '_'; break;
                case '=': key = '+'; break;
                case '[': key = '{'; break;
                case ']': key = '}'; break;
                case ';': key = ':'; break;
                case '\'': key = '"'; break;
                case ',': key = '<'; break;
                case '.': key = '>'; break;
                case '/': key = '?'; break;
                case '`': key = '~'; break;
            }
        }
        key_buffer_push(key);
    } else {
        if (inputting) {
            switch (key) {
                case KEY_BACKSPACE:
                    key_buffer_pop();
                    put_backspace();
                    break;
                case KEY_ENTER:
                    key_buffer_push(key);
                    break;
                case KEY_LEFT:
                    cursor_back();
                    break;
                case KEY_RIGHT:
                    cursor_forwrad();
                    break;
                //case KEY_UP:
                //    cursor_up();
                //    break;
                //case KEY_DOWN:
                //    cursor_down();
                //    break;
            }
        }
    }
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
        on_key_down(key);
    }
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
    char* arg = (char*)&fmt;
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
