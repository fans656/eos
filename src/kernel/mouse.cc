#include "mouse.h"
#include "graphics.h"
#include "stdio.h"
#include "asm.h"

extern int screen_width, screen_height;

constexpr uchar BTN_LEFT = 0x01;
constexpr uchar BTN_RIGHT = 0x02;
constexpr uchar BTN_MIDDLE = 0x04;
constexpr uchar X_SIGN = 0x10;
constexpr uchar Y_SIGN = 0x20;

bool mouse_left = false;
bool mouse_right = false;
int mouse_x, mouse_y;

static inline void mouse_read_wait() {
    while (!(inb(0x64) & 1)) {
        continue;
    }
}

static inline void mouse_write_wait() {
    while (inb(0x64) & 2) {
        continue;
    }
}

static inline uchar mouse_command(uchar cmd) {
    mouse_write_wait();
    outb(0x64, 0xd4);  // command byte
    mouse_write_wait();
    outb(0x60, cmd);  // data byte
    mouse_read_wait();
    uchar ack = inb(0x60);  // ack from mouse
    return ack;
}

// http://forum.osdev.org/viewtopic.php?t=10247
// http://wiki.osdev.org/Mouse_Input
// http://wiki.osdev.org/PS/2_Mouse
void init_mouse(int screen_width, int screen_height) {
    uchar status;

    mouse_write_wait();  // enable aux mouse device
    outb(0x64, 0xa8);
    
    mouse_write_wait();  // get compaq status
    outb(0x64, 0x20);
    mouse_read_wait();
    status = inb(0x60);

    status |= 0x02;  // enable IRQ12
    status &= ~0x20;  // disable mouse clock
    
    mouse_write_wait();  // set compaq status
    outb(0x64, 0x60);
    mouse_write_wait();
    outb(0x60, status);

    mouse_command(0xf6);  // set defaults
    mouse_command(0xf4);  // enable data reporting
    inb(0x60);
    
    mouse_x = screen_width / 2;
    mouse_y = screen_height / 2;
}

MouseEvent* parse_mouse_event(uchar* b) {
    mouse_left = (b[0] & BTN_LEFT) != 0;
    mouse_right = (b[0] & BTN_RIGHT) != 0;

    int dx = (((int)b[1]) & 0xff);
    if (b[0] & X_SIGN) dx = -((0xff - dx) + 1);

    int dy = ((int)b[2]) & 0xff;
    if (!(b[0] & Y_SIGN)) dy = -dy;
    else dy = 0xff - dy + 1;

    int new_mouse_x = mouse_x + dx;
    if (0 <= new_mouse_x && new_mouse_x < screen_width) {
        mouse_x = new_mouse_x;
    }
    int new_mouse_y = mouse_y + dy;
    if (0 <= new_mouse_y && new_mouse_y < screen_height) {
        mouse_y = new_mouse_y;
    }
    return new MouseEvent(mouse_x, mouse_y, (b[0] & BTN_LEFT) | (b[0] & BTN_RIGHT));
}
