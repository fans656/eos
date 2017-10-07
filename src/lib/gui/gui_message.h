#ifndef GUI_MESSAGE_H
#define GUI_MESSAGE_H

struct Window;

enum Type {
    WM_Create,
    WM_Show,
    WM_Update,
    WM_Move,
    WM_Resize,
    WM_Painted,

    WE_OnCreate,
    WE_OnMove,
    WE_OnSize,
    WE_OnPaint,
};

struct Message {
    Type type;
    Message(Type type) : type(type) {}
};

////////////////////////////////////////////////////////// WindowMessage

struct WindowMessage : public Message {
    WindowMessage(Type type, Window* wnd) : Message(type), wnd(wnd) {}
    Window* wnd;
};

struct WMCreate : public WindowMessage {
    WMCreate(Window* wnd) : WindowMessage(WM_Create, wnd) {}
};

struct WMShow : public WindowMessage {
    WMShow(Window* wnd) : WindowMessage(WM_Show, wnd) {}
};

struct WMUpdate : public WindowMessage {
    WMUpdate(Window* wnd) : WindowMessage(WM_Update, wnd) {}
};

struct WMMove : public WindowMessage {
    WMMove(Window* wnd, int x, int y) : WindowMessage(WM_Move, wnd), x(x), y(y) {}
    int x, y;
};

struct WMResize : public WindowMessage {
    WMResize(Window* wnd, int width, int height)
        : WindowMessage(WM_Resize, wnd), width(width), height(height) {}
    int width, height;
};

struct WMPainted : public WindowMessage {
    WMPainted(Window* wnd) : WindowMessage(WM_Painted, wnd) {}
};

////////////////////////////////////////////////////////// EventMessage

struct EventMessage : public Message {
    EventMessage(Type type) : Message(type) {}
};

struct WEOnCreate : public EventMessage {
    WEOnCreate() : EventMessage(WE_OnCreate) {}
};

struct WEOnMove : public EventMessage {
    WEOnMove(int x, int y) : EventMessage(WE_OnMove), x(x), y(y) {}
    int x, y;
};
typedef WEOnMove MoveEvent;

struct WEOnSize : public EventMessage {
    WEOnSize(int width, int height)
        : EventMessage(WE_OnSize), width(width), height(height)
    {}

    int width;
    int height;
};
typedef WEOnSize SizeEvent;

struct WEOnPaint : public EventMessage {
    WEOnPaint() : EventMessage(WE_OnPaint) {}
};
typedef WEOnPaint PaintEvent;

#endif
