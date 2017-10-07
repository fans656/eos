#ifndef WINDOW_H
#define WINDOW_H

#include "gui_message.h"

struct Surface;

struct Window {
    Window();
    ~Window();
    
    int left() const { return left_ + margin_left; }
    int top() const { return top_ + margin_top; }
    int right() const { return width_ - margin_right; }
    int bottom() const { return height_ - margin_bottom; }
    int width() const { return width_ - margin_left - margin_right; }
    int height() const { return height_ - margin_top - margin_bottom; }
    
    int frame_left() const { return left_; }
    int frame_top() const { return top_; }
    int frame_width() const { return width_; }
    int frame_height() const { return height_; }
    
    void move(int x, int y);
    void resize(int width, int height);
    
    virtual void on_event(EventMessage* ev);
    virtual void on_create();
    virtual void on_move(MoveEvent* ev);
    virtual void on_size(SizeEvent* ev);
    virtual void on_paint(PaintEvent* ev);

    void on_system_paint(PaintEvent* ev);

    bool destroyed() { return false; }
    void exec();
    
    void set_pos(int x, int y) {
        left_ = x;
        top_ = y;
    }

    void set_client_size(int width, int height) {
        set_size(width + margin_left + margin_right,
                height + margin_top + margin_bottom);
    }
    
    void set_size(int width, int height) {
        width_ = width;
        height_ = height;
    }

    Surface* surface;
    
    int left_, top_;
    int width_, height_;
    int margin_left, margin_right, margin_top, margin_bottom;
};

void gui_exec(Window* wnd);

#endif
