#include "gui.h"
#include "graphics.h"
#include "memory.h"
#include "stdio.h"
#include "stdlib.h"
#include "list.h"
#include "process.h"
#include "keyboard.h"

List<Window*> top_wnds;
List<Window*> event_waiting_wnds;

void wnd_draw(Window* wnd) {
    printf("draw window %x\n", wnd);
}

int register_window(Window* wnd) {
    wnd->proc = running_proc;
    wnd->paint_state = Unpainted;
    top_wnds.append(wnd);
}

Event* get_event(Window* wnd) {
    if (wnd->events.empty()) {
        event_waiting_wnds.append(wnd);
        process_make_event_waiting();
        return 0;
    } else {
        return wnd->events.popleft();
    }
}

static bool screen_dirty;

void gui_work() {
    if (screen_dirty) {
        bool has_unpainted = false;
        for (auto it = top_wnds.rbegin(); it != top_wnds.rend(); ++it) {
            Window* wnd = *it;
            if (wnd->paint_state == Painted) {
                wnd_draw(wnd);
            } else if (wnd->paint_state != Painting) {
                wnd->events.append(new PaintEvent);
                wnd->paint_state = Painting;
                process_wake_event_waiting(wnd->proc);
                has_unpainted = true;
            }
        }
        if (!has_unpainted) {
            screen_dirty = false;
        }
    }
}

int onkey(uint key, bool up) {
    for (Window* wnd: top_wnds) {
        wnd->events.append(new KeyboardEvent(key, up));
        process_wake_event_waiting(wnd->proc);
    }
}

void init_gui() {
    screen_dirty = true;
    top_wnds.construct();
    event_waiting_wnds.construct();
    listen_keyboard(onkey);
}
