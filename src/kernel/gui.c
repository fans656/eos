#include "gui.h"
#include "graphics.h"
#include "memory.h"
#include "stdio.h"
#include "stdlib.h"
#include "list.h"
#include "process.h"
#include "keyboard.h"

extern Process running_proc;

//////////////////////////////////////////////////////////////// event

PaintEvent paint_event_new() {
    PaintEvent ev = malloc(sizeof(PaintEvent));
    ev->type = PaintEventType;
    return ev;
}

KeyboardEvent keyboard_event_new(uint key, bool up) {
    KeyboardEvent ev = malloc(sizeof(KeyboardEvent));
    ev->type = KeyboardEventType;
    ev->key = key;
    ev->up = up;
    return ev;
}

//////////////////////////////////////////////////////////////// window

void wnd_draw(Window wnd) {
    printf("draw window %x\n", wnd);
}

//////////////////////////////////////////////////////////////// misc

List top_wnds;
List event_waiting_wnds;

int register_window(Window wnd) {
    wnd->events = list_new();
    wnd->proc = running_proc;
    wnd->paint_state = Unpainted;
    list_append(top_wnds, wnd);
}

Event get_event(Window wnd) {
    if (list_empty(wnd->events)) {
        list_append(event_waiting_wnds, wnd);
        process_make_event_waiting();
        return 0;
    } else {
        return (Event)list_popleft(wnd->events);
    }
}

static bool screen_dirty;

void gui_work() {
    if (screen_dirty) {
        bool has_unpainted = false;
        ListIter iter = list_riter(top_wnds);
        while (list_iter_valid(iter)) {
            Window wnd = (Window)iter->data;
            if (wnd->paint_state == Painted) {
                wnd_draw(wnd);
            } else if (wnd->paint_state != Painting) {
                list_append(wnd->events, paint_event_new());
                wnd->paint_state = Painting;
                process_wake_event_waiting(wnd->proc);
                has_unpainted = true;
            }
            iter = list_iter_prev(iter);
        }
        if (!has_unpainted) {
            screen_dirty = false;
        }
    }
}

int onkey(uint key, bool up) {
    ListIter iter = list_iter(top_wnds);
    while (list_iter_valid(iter)) {
        Window wnd = (Window)iter->data;
        list_append(wnd->events, keyboard_event_new(key, up));
        process_wake_event_waiting(wnd->proc);
        iter = list_iter_next(iter);
    }
}

void init_gui() {
    screen_dirty = true;
    top_wnds = list_new();
    event_waiting_wnds = list_new();
    listen_keyboard(onkey);
}
