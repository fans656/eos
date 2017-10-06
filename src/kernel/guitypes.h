#ifndef GUITYPES_H
#define GUITYPES_H

#include "../common/def.h"
#include "list.h"

constexpr int DEFAULT_WINDOW_WIDTH = 320;
constexpr int DEFAULT_WINDOW_HEIGHT = 240;

////////////////////////////////////////////////////////// enum

enum EventType {
    KeyboardEventType, PaintEventType,
};

////////////////////////////////////////////////////////// event

struct Event {
    EventType type;
};

struct PaintEvent_ : Event {
    PaintEvent_() { type = PaintEventType; }
};
typedef PaintEvent_& PaintEvent;

extern const char* VK2NAME[];

struct KeyboardEvent_ : Event {
    uint key;
    bool up;
    KeyboardEvent_(uint key, bool up) : key(key), up(up) { type = KeyboardEventType; }
    const char* name() { return VK2NAME[key]; }
};
typedef KeyboardEvent_& KeyboardEvent;

////////////////////////////////////////////////////////// canvas

class Window;

class Canvas_ {
public:
    Canvas_(Window* wnd);

    void fill_rect(int left, int top, int width, int height) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                draw_pixel(left + x, top + y, brush_color);
            }
        }
    }
    
    void draw_pixel(int x, int y, uint color) {
        uchar* p = buffer + y * pitch + x * bpp;
        *p++ = color >> 16;
        *p++ = (color >> 8) & 0xff;
        *p++ = color & 0xff;
    }
    
    int pitch;
    int bpp;
    uint brush_color;

private:
    uchar* buffer;
};
typedef Canvas_& Canvas;

////////////////////////////////////////////////////////// window

typedef struct _Process* Process;

class Window {
public:
    enum PaintState { Unpainted, Painting, Painted };
    
    int width() const { return width_; }
    int height() const { return height_; }
    int left() const { return left_; }
    int top() const { return top_; }
    
    Window() : Window(0, 0, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) {}

    Window(int left, int top, int width, int height) {
        left_ = left;
        top_ = top;
        width_ = width;
        height_ = height;
    }
    
    ~Window() {
        delete buffer;
    }
    
    void move(int x, int y) {
        left_ = x;
        top_ = y;
    }
    
    void resize(int width, int height) {
        width_ = width;
        height_ = height;
    }
    
    virtual void on_create() {
    }
    
    virtual void on_paint(PaintEvent ev) {
        printf("Window::on_paint() | %x\n", this);
    }
    
    virtual void on_key(KeyboardEvent ev) {
        if (ev.up) {
            on_key_up(ev);
        } else {
            on_key_down(ev);
        }
    }
    
    virtual void on_key_up(KeyboardEvent ev) {
        printf("Window-%x::on_key_up(%s)\n", this, ev.name());
    }

    virtual void on_key_down(KeyboardEvent ev) {
        printf("Window-%x::on_key_down(%s)\n", this, ev.name());
    }
    
    void init();
    void set_painted() { paint_state = Painted; }

    Event* get_event();
    void post_event(Event* ev) { events.append(ev); }

    Process process() { return proc; }
    Canvas canvas() { return *canvas_; }
    
    bool try_paint();
    void do_paint();
    
    Canvas_* canvas_;
    uchar* buffer;

private:
    int width_;
    int height_;
    int left_;
    int top_;

    Process proc;
    List<Event*> events;
    PaintState paint_state;
};

typedef uint (*WndProc)(Event*);

#endif
