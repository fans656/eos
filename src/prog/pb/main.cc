#include "stdio.h"
#include "gui.h"

struct Wnd : Window {
    void on_create() {
        resize(600, 100);
        move(100, 400);
    }
    
    void on_paint(PaintEvent ev) {
        Canvas c = canvas();
        c.fill_rect(0, 0, width(), height());
    }
};

int main() {
    Wnd* wnd = new Wnd;
    gui_main(wnd);
}
