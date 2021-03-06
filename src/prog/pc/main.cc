#include "stdio.h"
#include "stdlib.h"
#include "gui.h"
#include "eos.h"
#include "unistd.h"
#include "algorithm.h"
#include "time.h"

struct Wnd : public Window {
    Wnd() : Window(0,0,0,0, WND_TRANSPARENT) {
        img = new Bitmap("/img/walle.png");
        move(800, 100);
        resize(img->width(), img->height());
    }
    
    ~Wnd() { delete img; }
    
    void on_paint(PaintEvent* ev) {
        Painter painter(this);
        painter.draw_bitmap(0, 0, img);
    }
    
    Bitmap* img;
};

int main() {
    Wnd* wnd = new Wnd();
    wnd->exec();
}
