#include "stdio.h"
#include "stdlib.h"
#include "eos.h"
#include "gui.h"
#include "time.h"

struct Wnd : public Window {
    Wnd(int width, int height, uint attr)
        : Window(0, 0, width, height, attr) {
        background = new Bitmap("/img/snow-leopard.bmp");
    }
    
    ~Wnd() {
        delete background;
    }
    
    void on_paint(PaintEvent* ev) {
        Painter painter(this);
        painter.draw_bitmap(0, 0, background);
    }
    
    Bitmap* background;
};

int main() {
    auto info = new ScreenInfo;
    get_screen_info(info);
    Wnd* wnd = new Wnd(info->width, info->height,
            WND_CLIENT_ONLY | WND_KEEP_INACTIVE | WND_KEEP_BOTTOM);
    delete info;
    wnd->exec();
    delete wnd;
}
