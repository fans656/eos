#include "keyboard.h"
#include "list.h"
#include "stdio.h"

// http://www.computer-engineering.org/ps2keyboard/scancodes1.html

uint SCAN2VK[128] = {
    0,
    VK_Esc,'1','2','3','4','5','6','7','8','9','0','-','=',VK_Backspace,  // 0x01-0x0e
    VK_Tab,'q','w','e','r','t','y','u','i','o','p','[',']',VK_Enter,  // 0x0f-0x1c
    VK_LCtrl,'a','s','d','f','g','h','j','k','l',';','\'','`',  // 0x1d-0x29
    VK_LShift,'\\','z','x','c','v','b','n','m',',','.','/',VK_RShift,0,  // 0x2a-0x37
    VK_LAlt,' ',VK_Cap,VK_F1,VK_F2,VK_F3,VK_F4,VK_F5,VK_F6,VK_F7,VK_F8,VK_F9,VK_F10,  // 0x38-0x44
    0,0,VK_Home,VK_Up,0,0,VK_Left,0,VK_Right,0,VK_End,VK_Down,VK_PageDown,VK_Insert,VK_Delete,0,0,0, // 0x45-0x56
    VK_F11,VK_F12,
};

const char* VK2NAME[256] = {
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?",
    "@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
    "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_",
    "`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
    "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "",
    "VK_Left",
    "VK_Right",
    "VK_Up",
    "VK_Down",
    "VK_LCtrl",
    "VK_RCtrl",
    "VK_LShift",
    "VK_RShift",
    "VK_LAlt",
    "VK_RAlt",
    "VK_Tab",
    "VK_Cap",
    "VK_PrtScr",
    "VK_PageUp",
    "VK_PageDown",
    "VK_Home",
    "VK_End",
    "VK_Insert",
    "VK_Delete",
    "VK_Backspace",
    "VK_Enter",
    "VK_Esc",
    "VK_F1",
    "VK_F2",
    "VK_F3",
    "VK_F4",
    "VK_F5",
    "VK_F6",
    "VK_F7",
    "VK_F8",
    "VK_F9",
    "VK_F10",
    "VK_F11",
    "VK_F12",
};

static List<KeyboardListener> listeners;

void init_keyboard() {
    listeners.construct();
}

void listen_keyboard(KeyboardListener listener) {
    listeners.append(listener);
}

void update_key_state(uchar scancode) {
    bool up = scancode & 0x80;
    scancode &= 0x7f;
    uchar vk = SCAN2VK[scancode];
    for (auto onkey: listeners) {
        onkey(vk, up);
    }
}
