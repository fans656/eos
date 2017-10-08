#include "stdio.h"
#include "gui.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        img = new Bitmap("/img/girl.bmp");
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        c.draw_bitmap(img, (width() - img->width()) / 2, -25);
    }
    
    Bitmap* img;
};

int main() {
    Wnd* wnd = new Wnd;
    wnd->move(100, 200);
    wnd->resize(400, 300);
    gui_exec(wnd);
}
