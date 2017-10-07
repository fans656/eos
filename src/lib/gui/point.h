#ifndef POINT_H
#define POINT_H

struct Point {
    Point(int x, int y) : x_(x), y_(y) {}
    
    inline int x() const { return x_; }
    inline int y() const { return y_; }
    
    inline void set_x(int x) { x_ = x; }
    inline int set_y(int y) { y_ = y; }

    int x_, y_;
};

#endif
