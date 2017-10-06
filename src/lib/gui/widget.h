#ifndef WIDGET_H
#define WIDGET_H

struct Widget {
    Widget(Widget* parent, int left, int top, int width, int height)
        : parent(parent), left_(left), top_(top), width_(width), height_(height) {
    }
    
    int left() const { return left_; }
    int top() const { return top_; }
    int right() const { return left_ + width_; }
    int bottom() const { return top_ + height_; }
    
    Widget* parent;
    int left_, top_, width_, height_;
};

#endif
