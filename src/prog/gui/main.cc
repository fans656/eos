#include "gui.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "eos.h"
#include "time.h"
#include "list.h"
#include "pair.h"
#include "iterator.h"
#include "algorithm.h"

constexpr uint LEFT_BUTTON = 0x01;
constexpr uint RIGHT_BUTTON = 0x02;

#define left_button(buttons) ((buttons) & LEFT_BUTTON)
#define right_button(buttons) ((buttons) & RIGHT_BUTTON)

int screen_width;
int screen_height;
int screen_bpp;
int screen_pitch;
uchar* video_mem;

struct Server {
    Server() {
        mouse_img = new Bitmap("/img/mouse.png");
        mouse_rc.set_width(mouse_img->width());
        mouse_rc.set_height(mouse_img->height());
        screen = new Bitmap(screen_width, screen_height);
        video = new Bitmap(screen_width, screen_height, video_mem);
        video_painter = new Painter(video);
    }
    
    ~Server() {
        delete screen;
        delete video;
        delete video_painter;
    }

    void exec() {
        while (true) {
            auto msg = (WindowRequest*)get_message(QUEUE_ID_GUI);
            switch (msg->type) {
                case MOUSE_EVENT:
                    on_mouse((MouseEvent*)msg);
                    break;
                case CREATE:
                    create(msg->wnd);
                    break;
                case PAINTED:
                    painted(msg->wnd->swnd);
                    break;
                default:
                    printf("gui server: unknown message");
                    break;
            }
            delete msg;
        }
    }
    
    void on_mouse(MouseEvent* ev) {
        int x = ev->x;
        int y = ev->y;
        uint buttons = ev->buttons;
        
        if (buttons != mouse_buttons) {
            auto left = left_button(buttons);
            auto right = right_button(buttons);
            auto prev_left = left_button(mouse_buttons);
            auto prev_right = right_button(mouse_buttons);
            if (left && !prev_left) {
                on_mouse_press(ev, LEFT_BUTTON);
            } else if (right && !prev_right) {
                on_mouse_press(ev, RIGHT_BUTTON);
            } else if (!left && prev_left) {
                on_mouse_release(ev, LEFT_BUTTON);
            } else if (!right && prev_right) {
                on_mouse_release(ev, RIGHT_BUTTON);
            }
            mouse_buttons = buttons;
        }

        invalidate_mouse(false);
        mouse_rc.set_left(x);
        mouse_rc.set_top(y);
        invalidate_mouse(true);
    }
    
    void on_mouse_press(MouseEvent* ev, uint button) {
        printf("press %d\n", button);
    }
    
    void on_mouse_release(MouseEvent* ev, uint button) {
        printf("release %d\n", button);
    }
    
    void invalidate_mouse(bool draw) {
        List<Rect> rcs;
        rcs.append(mouse_rc);
        invalidate(rcs, draw);
    }
    
    void create(Window* uwnd) {
        auto wnd = new ServerWindow(uwnd);
        if (wnd->keep_bottom()) {
            wnds.prepend(wnd);
        } else {
            wnds.append(wnd);
        }
        wnd->create();
        bool painted = false;
        if (!wnd->keep_inactive()) {
            painted = activate(wnd);
        }
        if (!painted) {
            wnd->paint();
        }
    }
    
    void painted(ServerWindow* wnd) {
        List<Rect> rcs;
        rcs.append(wnd->window_rect_in_screen_coord());
        invalidate(rcs);
    }
    
    bool activate(ServerWindow* wnd) {
        auto pwnd = current_active_window();
        if (pwnd) {
            pwnd->deactivate();
            pwnd->paint();
        }
        bool painted = false;
        if (!wnd->keep_inactive()) {
            wnd->activate();
            put_at_top(wnd);
            wnd->paint();
            painted = true;
        }
        return painted;
    }
    
    ServerWindow* current_active_window() {
        for (auto wnd: wnds) {
            if (wnd->active()) {
                return wnd;
            }
        }
        return 0;
    }
    
    void put_at_top(ServerWindow* wnd) {
        wnds.remove(wnd);
        wnds.append(wnd);
    }
    
    void invalidate(List<Rect>& rcs, bool draw_mouse = true) {
        auto update_rcs = rcs.clone();
        List<ServerWindow*> wnds_to_draw;
        int cnt = 0;
        for (auto wnd: reversed(wnds)) {
            int size = rcs.size();
            if (size == 0) break;
            for (int i = 0; i < size; ++i) {
                if (wnd->clip(rcs)) {
                    wnds_to_draw.append(wnd);
                }
            }
        }
        Painter painter(screen);
        for (auto wnd: reversed(wnds_to_draw)) {
            wnd->blit(painter);
        }
        if (draw_mouse) {
            painter.draw_bitmap(mouse_rc, mouse_img, mouse_img->rect());
        }
        for (const auto& rc: update_rcs) {
            update_screen(rc);
        }
    }
    
    void update_screen(const Rect& rc) {
        video_painter->draw_bitmap(rc, screen, rc);
    }
    
    List<ServerWindow*> wnds;
    Bitmap* screen;
    Bitmap* video;
    Bitmap* mouse_img;
    Painter* video_painter;
    Rect mouse_rc;
    uint mouse_buttons;
};

int main() {
    auto info = new ScreenInfo;
    get_screen_info(info);
    screen_width = info->width;
    screen_height = info->height;
    screen_bpp = info->bpp;
    screen_pitch = info->pitch;
    video_mem = info->video_mem;
    Server server;
    server.exec();
}
