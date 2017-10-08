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
    margin_left_ = DEF_MARGIN_LEFT;
    margin_right_ = DEF_MARGIN_RIGHT;
    margin_top_ = DEF_MARGIN_TOP;
    margin_bottom_ = DEF_MARGIN_BOTTOM;
}

Window::~Window() {
    delete surface;
}

void Window::move(int x, int y) {
    put_message(GUI_MESSAGE_ID, new WMMove(this, x, y));
}

void Window::resize(int width, int height) {
    put_message(GUI_MESSAGE_ID, new WMResize(this,
                width + margin_left_ + margin_right_,
                height + margin_top_ + margin_bottom_));
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
    surface = new Surface(frame_width(), frame_height(), 4);
    put_message(GUI_MESSAGE_ID, new WMUpdate(this));
}

void Window::on_system_paint(PaintEvent* ev) {
    auto frame_color = SteelBlue;
    surface->fill_rect(0, 0, frame_width(), margin_top(), frame_color);  // top
    surface->fill_rect(0, margin_top(), margin_left(), height(), frame_color);  // left
    surface->fill_rect(frame_width() - margin_right(), margin_top(),
            margin_right(), height(), frame_color);  // right
    surface->fill_rect(0, frame_height() - margin_bottom(),
            frame_width(), margin_bottom(), frame_color);  // bottom
    surface->fill_rect(margin_left(), margin_top(),
            width(), height(), LightSteelBlue);  // client
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
