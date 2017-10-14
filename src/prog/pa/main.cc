#include "stdio.h"
#include "stdlib.h"
#include "gui.h"
#include "eos.h"
#include "unistd.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        img = new Bitmap("/img/png.png");
        resize(img->width(), img->height());
        timer1 = set_timer(1000);
        timer2 = set_timer(2000);
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        c.draw_bitmap(img, (width() - img->width()) / 2, (height() - img->height()) / 2);
    }
    
    void on_timer(TimerEvent* ev)  {
        printf("timer %d\n", ev->id);
    }
    
    Bitmap* img;
    uint timer1, timer2;
};

int main() {
    Wnd* wnd = new Wnd;
    wnd->set_attribute(WINDOW_CAPTION, false);
    wnd->set_attribute(WINDOW_FRAME, false);
    //wnd->set_attribute(WINDOW_TRANSPARENT, true);
    wnd->move(100, 100);
    gui_exec(wnd);
}
