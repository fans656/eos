#include "stdio.h"
#include "gui.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        move(100, 200);
        resize(400, 300);
        img = new Bitmap("/img/girl.bmp");
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        printf("%d %d %d\n", width(), img->width(), (width() - img->width()) / 2);
        c.draw_bitmap(img, (width() - img->width()) / 2, 0);
    }
    
    Bitmap* img;
};

int main() {
    gui_exec(new Wnd);
}
