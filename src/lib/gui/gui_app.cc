#include "gui_app.h"
#include "gui_message.h"
#include "eos.h"
#include "list.h"
#include "stdio.h"
#include "window.h"

void App::add(Window* wnd) {
    wnd->set_app(this);
    wnds.append(wnd);
}

void App::exec() {
    for (auto wnd: wnds) {
        put_message(GUI_MESSAGE_ID, new CreateWindowMessage(wnd));
    }
    while (!finished()) {
        WindowEvent* ev = (WindowEvent*)get_message((int)this);
        for (auto wnd: wnds) {
            if (wnd == ev->wnd) {
                wnd->on_event(ev);
            }
        }
        delete ev;
    }
}

void gui(Window* wnd) {
    App app;
    app.add(wnd);
    wnd->show();
    app.exec();
}
