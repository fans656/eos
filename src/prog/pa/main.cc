#include "gui.h"

struct Wnd : Window {
    void on_create() {
        move(20, 30);
    }

    void on_paint(PaintEvent ev) {
        auto c = canvas();
        c.brush_color = 0x81DAF5;
        c.fill_rect(0, 0, width(), height());
    }
};

int main() {
    Wnd* wnd = new Wnd;
    gui_main(wnd);
}
