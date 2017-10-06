#ifndef WINDOW_H
#define WINDOW_H

#include "surface.h"
#include "event.h"
#include "stdio.h"
#include "eos.h"
#include "gui_message.h"

struct App;

struct Window {
    Window() {
    }
    
    void show() {}
    
    virtual void on_event(WindowEvent* ev) {
        switch (ev->type) {
            case CreateEventType:
                on_create();
                break;
            case PaintEventType:
                on_paint((PaintEvent*)ev);
                put_message(GUI_MESSAGE_ID, new WindowPaintedMessage(this));
                break;
        }
    }
    
    virtual void on_create() {
        printf("Window-%x on_create\n");
    }
    
    virtual void on_paint(PaintEvent* ev) {
        printf("Window-%x on_paint\n");
    }
    
    bool destroyed() { return false; }
    
    Event* get_event() { return 0; }
    
    void set_app(App* app) { this->app = app; }

    Surface* surface;
    App* app;
};

struct Desktop : public Window {
    Desktop() {
    }
    
    virtual void on_paint(PaintEvent ev) {
        printf("Desktop::on_paint()\n");
    }
};

#endif
