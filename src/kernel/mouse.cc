#include "mouse.h"
#include "graphics.h"
#include "stdio.h"

extern int screen_width, screen_height;

constexpr uchar BTN_LEFT = 0x01;
constexpr uchar BTN_RIGHT = 0x02;
constexpr uchar BTN_MIDDLE = 0x04;
constexpr uchar X_SIGN = 0x10;
constexpr uchar Y_SIGN = 0x20;

bool mouse_left = false;
bool mouse_right = false;
int mouse_x, mouse_y;

void parse_mouse_event(uchar* b, GUIMouseEvent& ev) {
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
    ev.x = mouse_x;
    ev.y = mouse_y;
    ev.left = mouse_left;
    ev.right = mouse_right;
    //printf("%4d %4d %c%c\n", mouse_x, mouse_y,
    //        mouse_left ? 'L' : ' ', mouse_right ? 'R' : ' ');
}
