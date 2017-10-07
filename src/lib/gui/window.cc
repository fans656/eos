#include "window.h"
#include "eos.h"
#include "gui_message.h"
#include "surface.h"
#include "color.h"
#include "stdio.h"

constexpr int DEF_MARGIN_TOP = 20;
constexpr int DEF_MARGIN_LEFT = 2;
constexpr int DEF_MARGIN_RIGHT = 2;
constexpr int DEF_MARGIN_BOTTOM = 2;
constexpr int DEF_WND_CLIENT_WIDTH = 320;
constexpr int DEF_WND_CLIENT_HEIGHT = 240;

Window::Window() {
    set_pos(0, 0);
    set_client_size(DEF_WND_CLIENT_WIDTH, DEF_WND_CLIENT_HEIGHT);
    margin_left = DEF_MARGIN_LEFT;
    margin_right = DEF_MARGIN_RIGHT;
    margin_top = DEF_MARGIN_TOP;
    margin_bottom = DEF_MARGIN_BOTTOM;
}

Window::~Window() {
    delete surface;
}

void Window::move(int x, int y) {
    put_message(GUI_MESSAGE_ID, new WMMove(this, x, y));
}

void Window::resize(int width, int height) {
    put_message(GUI_MESSAGE_ID, new WMResize(this,
                width + margin_left + margin_right,
                height + margin_top + margin_bottom));
}

void Window::on_event(EventMessage* ev) {
    switch (ev->type) {
        case WE_OnCreate:
            on_create();
            break;
        case WE_OnMove:
            on_move((MoveEvent*)ev);
            break;
        case WE_OnSize:
            on_size((SizeEvent*)ev);
            break;
        case WE_OnPaint:
            on_system_paint((PaintEvent*)ev);
            on_paint((PaintEvent*)ev);
            put_message(GUI_MESSAGE_ID, new WMPainted(this));
            break;
    }
}

void Window::on_create() {
}

void Window::on_move(MoveEvent* ev) {
}

void Window::on_size(SizeEvent* ev) {
    if (surface) {
        delete surface;
    }
    surface = new Surface(frame_width(), frame_height(), 3);
    put_message(GUI_MESSAGE_ID, new WMUpdate(this));
}

void Window::on_system_paint(PaintEvent* ev) {
    surface->fill_rect(0, 0, frame_width(), margin_top, SteelBlue);  // top
    surface->fill_rect(0, margin_top, margin_left, height(), SteelBlue);  // left
    surface->fill_rect(frame_width() - margin_right, margin_top,
            margin_right, height(), SteelBlue);  // right
    surface->fill_rect(0, frame_height() - margin_bottom,
            frame_width(), margin_bottom, SteelBlue);  // bottom
}

void Window::on_paint(PaintEvent* ev) {
}

void Window::exec() {
    put_message(GUI_MESSAGE_ID, new WMCreate(this));
    while (!destroyed()) {
        EventMessage* ev = (EventMessage*)get_message((int)this);
        on_event(ev);
        delete ev;
    }
}

void gui_exec(Window* wnd) {
    wnd->exec();
}
