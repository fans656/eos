#ifndef RECT_H
#define RECT_H

#include "point.h"
#include "list.h"

struct Rect {
    Rect() : left_(0), top_(0), width_(0), height_(0) {}
    Rect(const Rect& rc)
        : left_(rc.left()), top_(rc.top_), width_(rc.width_), height_(rc.height_) {}
    Rect(int left, int top, int width, int height)
        : left_(left), top_(top), width_(width), height_(height) {}
    
    inline int left() const { return left_; }
    inline int top() const { return top_; }
    inline int right() const { return left_ + width_; }
    inline int bottom() const { return top_ + height_; }

    inline int width() const  { return width_; }
    inline int height() const  { return height_; }

    inline void set_left(int left) { left_ = left; }
    inline void set_top(int top) { top_ = top; }
    inline void set_right(int right) { left_ = right - width_; }
    inline void set_bottom(int bottom) { top_ = bottom - height_; }
    
    inline void set_width(int width) { width_ = width; }
    inline void set_height(int height) { height_ = height; }

    inline void adjust(int dleft, int dtop, int dright, int dbottom) {
        left_ += dleft;
        top_ += dtop;
        width_ += -dleft + dright;
        height_ += -dtop + dbottom;
    }
    
    inline void translate(int dx, int dy) { left_ += dx; top_ += dy; }
    inline Rect translated(int dx, int dy) {
        return Rect(left_ + dx, top_ + dy, width_, height_);
    }
    
    inline bool empty() const { return width_ <= 0 || height_ <= 0; }
    
    bool is_intersected(const Rect& rc);
    void intersect(const Rect& rc);
    void intersect(int left, int top, int width, int height);
    Rect intersected(const Rect& rc) const;
    
    List<Rect> operator-(const Rect& o) {
        List<Rect> res;
        Rect r(*this);
        if (o.left() > left()) {
            res.append(Rect(r.left(), r.top(), o.left() - r.left(), r.height()));
            r.set_left(o.left());
        }
        if (o.right() < r.right()) {
            res.append(Rect(o.right(), r.top(), r.right() - o.right(), r.height()));
            r.set_right(o.right());
        }
        if (o.top() > r.top()) {
            res.append(Rect(r.left(), r.top(), r.width(), o.top() - r.top()));
        }
        if (o.bottom() < r.bottom()) {
            res.append(Rect(r.left(), o.bottom(), r.width(), r.bottom() - o.bottom()));
        }
        return res;
    }
    
    Point center() const { return Point(left() + width() / 2, top() + height() / 2); }
    
    void clip_and_apply(const Rect& rc, Rect& buddy);
    
    void dump(const char* name);

    int left_, top_, width_, height_;
};

#endif
