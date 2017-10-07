#include "stdio.h"
#include "gui.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        move(50, 50);
        resize(400, 300);
    }
    
    void on_paint(PaintEvent* ev) {
        printf("user on_paint\n");
    }
};

int main() {
    gui_exec(new Wnd);
}
