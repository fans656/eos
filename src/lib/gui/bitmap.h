#ifndef BITMAP_H
#define BITMAP_H

struct Bitmap {
    Bitmap(const char* path);
    inline int width() const { return width_; }
    inline int height() const { return height_; }
    
    int width_, height_;
    int pitch;
    char* buffer;
};

#endif
