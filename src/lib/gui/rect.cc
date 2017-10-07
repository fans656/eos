#include "rect.h"
#include "math.h"

void Rect::intersect(const Rect& rc) {
    left_ = max(left(), rc.left());
    top_ = max(top(), rc.top());
    width_ = max(min(right(), rc.right()) - left_, 0);
    height_ = max(min(bottom(), rc.bottom()) - top_, 0);
}
