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
    
    inline bool empty() const { return width_ <= 0 || height_ <= 0; }
    
    inline void move_to(int x, int y) { left_ = x; top_ = y; }

    inline void adjust(int dleft, int dtop, int dright, int dbottom) {
        left_ += dleft;
        top_ += dtop;
        width_ += -dleft + dright;
        height_ += -dtop + dbottom;
    }
    
    inline Rect adjusted(int dleft, int dtop, int dright, int dbottom) {
        Rect rc(*this);
        rc.adjust(dleft, dtop, dright, dbottom);
        return rc;
    }

    inline void set_left(int left) {
        int dx = left - left_;
        left_ = left;
        width_ -= dx;
    }

    inline void set_top(int top) {
        int dy = top - top_;
        top_ = top;
        height_ -= dy;
    }

    inline void set_right(int right) { width_ += right - this->right(); }
    inline void set_bottom(int bottom) { height_ += bottom - this->bottom(); }
    
    inline void set_width(int width) { width_ = width; }
    inline void set_height(int height) { height_ = height; }
    
    inline bool contains(int x, int y) {
        return left() <= x && x < right() && top() <= y && y < bottom();
    }
    
    inline void translate(int dx, int dy) { left_ += dx; top_ += dy; }
    inline Rect translated(int dx, int dy) {
        return Rect(left_ + dx, top_ + dy, width_, height_);
    }
    
    bool is_intersected(const Rect& rc) const;
    void intersect(const Rect& rc);
    void intersect(int left, int top, int width, int height);
    Rect intersected(const Rect& rc) const;
    
    List<Rect> operator-(const Rect& o) const {
        List<Rect> res;
        if (!is_intersected(o)) {
            res.append(Rect(*this));
            return res;
        }
        Rect r(*this);
        if (o.top() > r.top()) {
            res.append(Rect(r.left(), r.top(), r.width(), o.top() - r.top()));
            r.set_top(o.top());
        }
        if (o.bottom() < r.bottom()) {
            res.append(Rect(r.left(), o.bottom(), r.width(), r.bottom() - o.bottom()));
            r.set_bottom(o.bottom());
        }
        if (o.left() > r.left()) {
            res.append(Rect(r.left(), r.top(), o.left() - r.left(), r.height()));
        }
        if (o.right() < r.right()) {
            res.append(Rect(o.right(), r.top(), r.right() - o.right(), r.height()));
        }
        //printf("~~~~~~~~~~~~~~~~~~~~~ Rect.-\n");
        //this->dump("this");
        //o.dump("o   ");
        //for (auto rc: res) {
        //    rc.dump("");
        //}
        return res;
    }
    
    List<Rect> operator+(const Rect& o) const {
        List<Rect> combined;
        Rect inter_rc = intersected(o);
        if (inter_rc.empty()) {
            combined.append(o);
        } else {
            combined.extend(o - inter_rc);
        }
        combined.append(*this);
        return combined;
    }
    
    Point center() const { return Point(left() + width() / 2, top() + height() / 2); }
    
    void dump(const char* name) const;

    int left_, top_, width_, height_;
};

void test_rect();

#endif
