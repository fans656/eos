#ifndef BITMAP_H
#define BITMAP_H

struct Bitmap {
    Bitmap(const char* path);
    
    int width, height;
    int pitch;
    char* buffer;
};

#endif
