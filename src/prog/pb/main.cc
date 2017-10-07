#include "stdio.h"
#include "gui.h"

struct Wnd : public Window {
    void on_create() {
        Window::on_create();
        move(20, 300);
        resize(50, 200);
    }
};

int main() {
    gui_exec(new Wnd);
}
