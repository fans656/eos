#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "def.h"

typedef enum Key {
    VK_Left = 128,
    VK_Right,
    VK_Up,
    VK_Down,
    VK_LCtrl,
    VK_RCtrl,
    VK_LShift,
    VK_RShift,
    VK_LAlt,
    VK_RAlt,
    VK_Tab,
    VK_Cap,
    VK_PrtScr,
    VK_PageUp,
    VK_PageDown,
    VK_Home,
    VK_End,
    VK_Insert,
    VK_Delete,
    VK_Backspace,
    VK_Enter,
    VK_Esc,
    VK_F1,
    VK_F2,
    VK_F3,
    VK_F4,
    VK_F5,
    VK_F6,
    VK_F7,
    VK_F8,
    VK_F9,
    VK_F10,
    VK_F11,
    VK_F12,
} Key;

typedef int (*KeyboardListener)(uint, bool);

void init_keyboard();
void listen_keyboard(KeyboardListener listener);
void update_key_state(uchar scancode);

#endif
