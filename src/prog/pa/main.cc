#include "stdio.h"
#include "stdlib.h"
#include "gui.h"
#include "eos.h"
#include "unistd.h"
#include "algorithm.h"
#include "time.h"

struct Wnd : public Window {
    Wnd() {
        img = new Bitmap("/img/girl.bmp");
        move(50, 125);
        resize(300, 200);
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
