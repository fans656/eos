#include "stdio.h"
#include "stdlib.h"
#include "gui.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        img = new Bitmap("/img/snow-leopard.bmp");
        imgs = new Bitmap*[3];
        imgs[0] = img;
        imgs[1] = new Bitmap("/img/cheetah.png");
        //imgs[2] = new Bitmap("/img/walle-desktop.png");
        resize(img->width(), img->height());
        //set_timer(3000);
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        c.draw_bitmap(img, 0, 0);
    }
    
    void on_timer(TimerEvent* ev) {
        i_img = (i_img + 1) % 2;
        img = imgs[i_img];
        update();
    }
    
    Bitmap* img;
    Bitmap** imgs;
    int i_img = 0;
};

int main() {
    Wnd* wnd = new Wnd;
    wnd->set_attribute(WINDOW_CAPTION, false);
    wnd->set_attribute(WINDOW_FRAME, false);
    wnd->move(0, 0);
    gui_exec(wnd);
}
