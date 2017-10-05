#include "gui.h"

uint wndproc(Event* ev) {
}

int main() {
    Window* wnd = (Window*)wnd_new(wndproc);
    gui_main(wnd);
}
