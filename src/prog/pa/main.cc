#include "stdio.h"
#include "gui.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        move(50, 50);
        resize(400, 300);
    }
    
    void on_paint(PaintEvent* ev) {
        Canvas c(this);
        auto rc = rect();
        auto pt = rc.center();
        c.fill_rect(pt.x(), pt.y(), 20, 20);
    }
};

int main() {
    gui_exec(new Wnd);
}
