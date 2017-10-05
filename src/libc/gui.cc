#include "gui.h"
#include "stdio.h"
#include "stdlib.h"

Window* wnd_new(WndProc wndproc) {
    Window* wnd = new Window;
    wnd->width = DEFAULT_WINDOW_WIDTH;
    wnd->height = DEFAULT_WINDOW_HEIGHT;
    wnd->x = 0;
    wnd->y = 0;
    return wnd;
}

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
            case KeyboardEventType: {
                KeyboardEvent* kev = (KeyboardEvent*)ev;
                printf("KeyboardEvent  key %x  %s\n", kev->key, kev->up ? "Up" : "Down");
                break;
                                    }
            case PaintEventType:
                printf("PaintEvent\n");
                break;
            default:
                printf("Event  type %d\n", ev->type);
                break;
        }
        free(ev);
    }
}
