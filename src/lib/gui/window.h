#ifndef WINDOW_H
#define WINDOW_H

#include "gui_message.h"
#include "rect.h"

struct Surface;

struct Window {
    Window();
    Window(int x, int y, int width, int height);
    ~Window();
    
    int left() const { return left_ + margin_left_; }
    int top() const { return top_ + margin_top_; }
    int right() const { return width_ - margin_right_; }
    int bottom() const { return height_ - margin_bottom_; }
    int width() const { return width_ - margin_left_ - margin_right_; }
    int height() const { return height_ - margin_top_ - margin_bottom_; }
    
    Rect rect() const { return Rect(0, 0, width(), height()); }
    
    int frame_left() const { return left_; }
    int frame_top() const { return top_; }
    int frame_right() const { return left_ + width_; }
    int frame_bottom() const { return top_ + height_; }
    int frame_width() const { return width_; }
    int frame_height() const { return height_; }

    int margin_left() const { return margin_left_; }
    int margin_right() const { return margin_right_; }
    int margin_top() const { return margin_top_; }
    int margin_bottom() const { return margin_bottom_; }
    
    void move(int x, int y);
    void resize(int width, int height);
    
    virtual void on_event(EventMessage* ev);
    virtual void on_create();
    virtual void on_move(MoveEvent* ev);
    virtual void on_size(SizeEvent* ev);
    virtual void on_paint(PaintEvent* ev);

    void on_system_paint(PaintEvent* ev);

    void init(int x, int y, int width, int height);
    bool destroyed() { return false; }
    void exec();
    
    void set_pos(int x, int y) {
        left_ = x;
        top_ = y;
    }

    void set_client_size(int width, int height) {
        set_size(width + margin_left_ + margin_right_,
                height + margin_top_ + margin_bottom_);
    }
    
    void set_size(int width, int height) {
        width_ = width;
        height_ = height;
    }

    Surface* surface;
    
    int left_, top_;
    int width_, height_;
    int margin_left_, margin_right_, margin_top_, margin_bottom_;
    bool created = false;
};

void gui_exec(Window* wnd);

#endif
