#include "gui.h"
#include "stdio.h"
#include "stdlib.h"

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

Event* _get_event(Window* wnd) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_GET_EVENT));
}

Event* get_event(Window* wnd) {
    Event* ev = 0;
    while (!ev) {
        ev = _get_event(wnd);
    }
    return ev;
}

int gui_main(Window* wnd) {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_REGISTER_WINDOW));
    while (true) {
        Event* ev = get_event(wnd);
        switch (ev->type) {
            case KeyboardEventType:
                wnd->on_key(*(KeyboardEvent_*)ev);
                break;
            case PaintEventType:
                wnd->on_paint(*(PaintEvent_*)ev);
                wnd->set_painted();
                break;
            default:
                printf("Event  type %d\n", ev->type);
                break;
        }
        free(ev);
    }
}
