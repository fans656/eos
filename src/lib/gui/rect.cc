#include "rect.h"
#include "math.h"
#include "stdio.h"

bool Rect::is_intersected(const Rect& rc) {
    if (rc.right() <= left()) return false;
    if (rc.left() >= right()) return false;
    if (rc.bottom() <= top()) return false;
    if (rc.top() >= bottom()) return false;
    return true;
}

void Rect::intersect(const Rect& rc) {
    if (is_intersected(rc)) {
        int l = max(rc.left(), left());
        int t = max(rc.top(), top());
        int r = min(rc.right(), right());
        int b = min(rc.bottom(), bottom());
        left_ = l;
        top_ = t;
        width_ = r - l;
        height_ = b - t;
    } else {
        width_ = height_ = 0;
    }
}

void Rect::intersect(int left, int top, int width, int height) {
    intersect(Rect(left, top, width, height));
}

Rect Rect::intersected(const Rect& rc) const {
    Rect res(*this);
    res.intersect(rc);
    return res;
}

void Rect::dump(const char* name) {
    printf("%s %d %d %d %d wh %d %d\n",
            name, left(), top(), right(), bottom(), width(), height());
}
