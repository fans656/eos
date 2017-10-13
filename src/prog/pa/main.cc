#include "stdio.h"
#include "stdlib.h"
#include "gui.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        //img = new Bitmap("/img/girl.bmp");
        img = new Bitmap("/img/girl2.png");
        resize(img->width(), img->height());
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        //c.draw_bitmap(img, (width() - img->width()) / 2, -25);
        c.draw_bitmap(img, 0, 0);
    }
    
    Bitmap* img;
};

int main() {
    Wnd* wnd = new Wnd;
    wnd->move(100, 100);
    gui_exec(wnd);
}
