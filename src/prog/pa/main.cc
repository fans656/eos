#include "stdio.h"
#include "stdlib.h"
#include "gui.h"
#include "eos.h"
#include "unistd.h"
#include "algorithm.h"
#include "time.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        img = new Bitmap("/img/girl-blue.png");
        resize(img->width(), img->height());
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        c.draw_bitmap(img, 0, 0);
    }
    
    Bitmap* img;
};

int main() {
    Wnd* wnd = new Wnd;
    wnd->set_attribute(WINDOW_CAPTION, false);
    wnd->set_attribute(WINDOW_FRAME, false);
    wnd->set_attribute(WINDOW_TRANSPARENT, true);
    wnd->move(100, 100);
    gui_exec(wnd);
}
