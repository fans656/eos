#ifndef EVENT_H
#define EVENT_H

struct Window;

enum EventType {
    CreateEventType,
    PaintEventType,
};

struct Event {
    EventType type;
};

struct WindowEvent : public Event {
    WindowEvent(Window* wnd) : wnd(wnd) {}
    
    Window* wnd;
};

struct CreateEvent : public WindowEvent {
    CreateEvent(Window* wnd) : WindowEvent(wnd) {
        type = CreateEventType;
    }
};

struct PaintEvent : public WindowEvent {
    PaintEvent(Window* wnd) : WindowEvent(wnd) {
        type = PaintEventType;
    }
};

#endif
