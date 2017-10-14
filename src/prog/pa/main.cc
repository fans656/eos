#include "stdio.h"
#include "stdlib.h"
#include "gui.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        img_bk = new Bitmap("/img/trans-back.png");
        img = new Bitmap("/img/png.png");
        resize(img_bk->width(), img_bk->height());
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        c.draw_bitmap(img_bk, 0, 0);
        c.draw_bitmap(img, (width() - img->width()) / 2, (height() - img->height()) / 2);
    }
    
    Bitmap* img;
    Bitmap* img_bk;
};

int main() {
    Wnd* wnd = new Wnd;
    wnd->move(200, 100);
    gui_exec(wnd);
}
