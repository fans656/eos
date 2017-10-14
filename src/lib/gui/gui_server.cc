#include "gui_server.h"
#include "gui_message.h"
#include "window.h"
#include "eos.h"
#include "list.h"
#include "stdio.h"
#include "surface.h"
#include "canvas.h"
#include "unistd.h"
#include "string.h"
#include "bitmap.h"
#include "png.h"
#include "stdlib.h"

GUIInfo* init_gui() {
    asm("mov eax, %0; int 0x80" :: "i"(SYSCALL_INIT_GUI));
}

struct Server {
    void init_gui() {
        GUIInfo* info = ::init_gui();
        screen_width = info->screen_width;
        screen_height = info->screen_height;
        screen_pitch = info->screen_pitch;
        screen_bpp = info->screen_bpp;
        delete info;
        
        screen = new Surface(screen_width, screen_height, screen_bpp);
        canvas = new Canvas(screen);
        
        mouse_x = screen_width / 2;
        mouse_y = screen_height / 2;
    }

    void exec() {
        init_gui();
        while (true) {
            bool worked = false;
            worked |= check_mouse();
            worked |= check_message();
            if (!worked) {
                sleep(1);
            }
        }
    }
    
    void draw_mouse(int x, int y) {
        //memory_blit(desktop, screen_pitch, mouse_x, mouse_y, mouse_x, mouse_y, 5, 5);
        //memory_blit(mouse_bitmap, 15, 0, 0, x, y, 5, 5);
    }
    
    bool check_mouse() {
        GUIMouseEvent* ev = (GUIMouseEvent*)get_message(GUI_MOUSE_EVENT_ID, false);
        if (ev == 0) return false;
        draw_mouse(ev->x, ev->y);
        mouse_x = ev->x;
        mouse_y = ev->y;
        return true;
    }
    
    bool check_message() {
        WindowMessage* msg = (WindowMessage*)get_message(GUI_MESSAGE_ID, false);
        if (!msg) return false;
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
        return true;
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
        composite(wnd);
        memory_blit(screen->buffer(), screen->pitch(),
                wnd->frame_left(), wnd->frame_top(),
                wnd->frame_left(), wnd->frame_top(),
                wnd->frame_width(), wnd->frame_height());
    }
    
    void composite(Window* wnd) {
        wnd->surface->switch_dst();
        canvas->blit(wnd->surface, wnd->frame_left(), wnd->frame_top(),
                wnd->frame_width(), wnd->frame_height());
    }

    List<Window*> top_wnds;

    int screen_width, screen_height;
    int screen_pitch, screen_bpp;
    int mouse_x, mouse_y;
    
    Surface* screen;
    Canvas* canvas;
};

void gui_server() {
    Server().exec();
}
