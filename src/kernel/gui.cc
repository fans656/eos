#include "gui.h"
#include "graphics.h"
#include "memory.h"
#include "stdio.h"
#include "stdlib.h"
#include "list.h"
#include "process.h"
#include "keyboard.h"
#include "util.h"
#include "graphics.h"

List<Window*> top_wnds;
List<Window*> event_waiting_wnds;

void wnd_draw(Window* wnd) {
    printf("draw window %x\n", wnd);
}

int register_window(Window* wnd) {
    wnd->init();
    wnd->on_create();
    top_wnds.append(wnd);
}

///////////////////////////////////////////////////////////// Canvas

Canvas_::Canvas_(Window* wnd) {
    brush_color = 0xffffff;
    buffer = wnd->buffer;
    bpp = screen_bpp;
    pitch = wnd->width() * bpp;
}

///////////////////////////////////////////////////////////// Window

void Window::init() {
    proc = running_proc;
    paint_state = Unpainted;
    size_t size = width() * height() * screen_bpp;
    buffer = new uchar[size];
    canvas_ = new Canvas_(this);
}

Event* Window::get_event() {
    if (events.empty()) {
        event_waiting_wnds.append(this);
        process_make_event_waiting();
        return 0;
    } else {
        return events.popleft();
    }
}

bool Window::try_paint() {
    switch (paint_state) {
        case Painted:
            do_paint();
            return true;
        case Unpainted:
            events.append(new PaintEvent_());
            process_wake_event_waiting(proc);
            paint_state = Painting;
            return false;
        case Painting:
            return false;
    }
    panic("Window::try_paint shouldn't be here");
}

void Window::do_paint() {
    int pitch = canvas_->pitch;
    int bpp = canvas_->bpp;
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            uchar* p = buffer + y * pitch + x * bpp;
            uint color = (*p << 16) | (*(p + 1) << 8) | *(p + 2);
            ::draw_pixel(x + left_, y + top_, color);
        }
    }
}

///////////////////////////////////////////////////////////// misc

static bool screen_dirty;

void gui_work() {
    if (screen_dirty) {
        screen_dirty = false;
        for (auto wnd: reversed(top_wnds)) {
            if (!wnd->try_paint()) {
                screen_dirty = true;
            }
        }
    }
}

int onkey(uint key, bool up) {
    for (Window* wnd: top_wnds) {
        wnd->post_event(new KeyboardEvent_(key, up));
        process_wake_event_waiting(wnd->process());
    }
}

void init_gui() {
    screen_dirty = true;
    top_wnds.construct();
    event_waiting_wnds.construct();
    listen_keyboard(onkey);
}
