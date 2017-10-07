#include "gui_server.h"
#include "gui_message.h"
#include "window.h"
#include "eos.h"
#include "list.h"
#include "graphics.h"
#include "stdio.h"
#include "surface.h"

struct Server {
    void exec() {
        while (true) {
            WindowMessage* msg = (WindowMessage*)get_message(GUI_MESSAGE_ID);
            if (!msg) continue;
            switch (msg->type) {
                case WM_Create:
                    on_create(msg->wnd);
                    break;
                case WM_Update:
                    on_update(msg->wnd);
                    break;
                case WM_Move:
                    on_move((WMMove*)msg);
                    break;
                case WM_Resize:
                    on_resize((WMResize*)msg);
                    break;
                case WM_Painted:
                    on_painted(msg->wnd);
                    break;
                default:
                    break;
            }
        }
    }

    void on_create(Window* wnd) {
        top_wnds.append(wnd);
        put_message((int)wnd, new WEOnCreate);
        put_message((int)wnd, new WEOnSize(wnd->width(), wnd->height()));
    }

    void on_update(Window* wnd) {
        put_message((int)wnd, new WEOnPaint);
    }
    
    void on_move(WMMove* msg) {
        auto wnd = msg->wnd;
        wnd->set_pos(msg->x, msg->y);
        put_message((int)wnd, new WEOnMove(msg->x, msg->y));
    }
    
    void on_resize(WMResize* msg) {
        auto wnd = msg->wnd;
        wnd->set_size(msg->width, msg->height);
        put_message((int)wnd, new WEOnSize(msg->width, msg->height));
    }
    
    void on_painted(Window* wnd) {
        memory_blit(wnd->surface->buffer, wnd->surface->pitch,
                0, 0, wnd->frame_left(), wnd->frame_top(),
                wnd->frame_width(), wnd->frame_height());
    }
    
    List<Window*> top_wnds;
};

void gui_server() {
    Server().exec();
}
