#include "stdio.h"
#include "stdlib.h"
#include "gui.h"
#include "eos.h"
#include "unistd.h"
#include "algorithm.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        img = new Bitmap("/img/png.png");
        img2 = new Bitmap("/img/walle.png");
        resize(500, 400);
        timer2 = set_timer(500);
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        c.draw_bitmap(img, x, (height() - img->height()) / 2);
    }
    
    void on_timer(TimerEvent* ev)  {
        x += 10;
        if (x + img->width() > width()) {
            swap(img, img2);
            x = 0;
        }
        update();
    }
    
    Bitmap* img;
    Bitmap* img2;
    uint timer1, timer2;
    int x;
};

int main() {
    Wnd* wnd = new Wnd;
    wnd->set_attribute(WINDOW_CAPTION, false);
    wnd->set_attribute(WINDOW_FRAME, false);
    wnd->set_attribute(WINDOW_TRANSPARENT, true);
    wnd->move(100, 100);
    gui_exec(wnd);
}
