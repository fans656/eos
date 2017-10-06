#include "stdio.h"
#include "gui.h"

struct Wnd : Window {
};

int main() {
    gui(new Wnd);
}
