#include "rect.h"
#include "math.h"
#include "stdio.h"

void Rect::intersect(const Rect& rc) {
    intersect(rc.left(), rc.top(), rc.width(), rc.height());
}

void Rect::intersect(int left, int top, int width, int height) {
    int r = left + width;
    int b = top + height;
    left_ = max(left_, left);
    top_ = max(top_, top);
    width_ = max(min(right(), r) - left_, 0);
    height_ = max(min(bottom(), b) - top_, 0);
}

void Rect::dump(const char* name) {
    printf("%s %d %d %d %d wh %d %d\n",
            name, left(), top(), right(), bottom(), width(), height());
}
