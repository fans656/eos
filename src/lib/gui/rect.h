#ifndef RECT_H
#define RECT_H

#include "point.h"

struct Rect {
    Rect(int left, int top, int width, int height)
        : left_(left), top_(top), width_(width), height_(height) {}
    
    inline int left() const { return left_; }
    inline int top() const { return top_; }
    inline int right() const { return left_ + width_; }
    inline int bottom() const { return top_ + height_; }
    inline int width() const  { return width_; }
    inline int height() const  { return height_; }
    
    inline void set_width(int width) { width_ = width; }
    inline void set_height(int height) { height_ = height; }
    
    inline void translate(int dx, int dy) { left_ += dx; top_ += dy; }
    
    void intersect(const Rect& rc);
    void intersect(int left, int top, int width, int height);
    
    Point center() const { return Point(left() + width() / 2, top() + height() / 2); }
    
    void clip_and_apply(const Rect& rc, Rect& buddy);
    
    void dump(const char* name);

    int left_, top_, width_, height_;
};

#endif
