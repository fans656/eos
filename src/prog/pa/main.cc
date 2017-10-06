#include "stdio.h"
#include "gui.h"

struct Wnd : public Window {
    Wnd() {
    }
};

int main() {
    gui(new Wnd);
}
