#ifndef GUI_MESSAGE_H
#define GUI_MESSAGE_H

#include "gui_app.h"

struct Window;

enum Type {
    CreateWindow, UpdateWindow, WindowPainted,
};

struct Message {
    Type type;
    Window* wnd;
    Message(Type type, Window* wnd) : type(type), wnd(wnd) {}
};

struct CreateWindowMessage : public Message {
    CreateWindowMessage(Window* wnd) : Message(CreateWindow, wnd) {}
};

struct UpdateWindowMessage : public Message {
    UpdateWindowMessage(Window* wnd) : Message(UpdateWindow, wnd) {}
};

struct WindowPaintedMessage : public Message {
    WindowPaintedMessage(Window* wnd) : Message(WindowPainted, wnd) {}
};

#endif
