#ifndef BITMAP_H
#define BITMAP_H

#include "def.h"
#include "rect.h"

struct Bitmap {
    Bitmap(const char* path);
    Bitmap(int width, int height);
    Bitmap(int width, int height, uchar* buffer);
    ~Bitmap() { if (own_) delete buffer_; }
    inline int width() const { return width_; }
    inline int height() const { return height_; }
    inline Rect rect() const { return Rect(0, 0, width(), height()); }
    inline int pitch() const { return pitch_; }
    inline bool opaque() const { return opaque_; }
    inline uchar* buffer() const { return buffer_; }
    
    void clear();
    
    int width_;
    int height_;
    int pitch_;
    uchar* buffer_;
    bool opaque_;
    bool own_;
};

#endif
