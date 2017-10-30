#include "gui.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "eos.h"
#include "time.h"
#include "list.h"
#include "array.h"
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
                case DESTROY:
                    destroy(msg->wnd->swnd);
                    break;
                case PAINTED:
                    painted(msg->wnd->swnd);
                    break;
                case UPDATE:
                    msg->wnd->swnd->paint();
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
        
        on_mouse_move(x, y, buttons);
        
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

        invalidate_mouse(mouse_rc, false);
        mouse_rc.move_to(x, y);
        invalidate_mouse(mouse_rc, true);
    }

    void on_mouse_move(int x, int y, uint buttons) {
        if (dragging_wnd) {
            auto old_rc = dragging_wnd->window_rect_in_screen_coord();
            int dx = x - dragging_initial_mouse_x;
            int dy = y - dragging_initial_mouse_y;
            if (!dx || !dy) return;
            int wnd_x = dragging_initial_window_x + dx;
            int wnd_y = dragging_initial_window_y + dy;
            dragging_wnd->move(wnd_x, wnd_y);
            auto new_rc = dragging_wnd->window_rect_in_screen_coord();
            auto rcs = old_rc + new_rc;
            invalidate(rcs);
        }
    }
    
    void on_mouse_press(MouseEvent* ev, uint button) {
        int x = ev->x;
        int y = ev->y;
        for (auto wnd: reversed(wnds)) {
            bool handled = false;
            if (wnd->hit_test_activate(x, y)) {
                activate(wnd);
                handled = true;
            }
            if (wnd->hit_test_drag(x, y)) {
                start_drag(wnd, x, y);
                handled = true;
            }
            if (wnd->client_rect_in_screen_coord().contains(x, y)) {
                wnd->mouse_press(ev);
                handled = true;
            }
            if (handled) break;
        }
    }
    
    void on_mouse_release(MouseEvent* ev, uint button) {
        for (auto wnd: reversed(wnds)) {
            bool handled = false;
            if (wnd->client_rect_in_screen_coord().contains(ev->x, ev->y)) {
                wnd->mouse_release(ev);
                handled = true;
            }
            if (handled) break;
        }
        stop_drag();
    }
    
    void start_drag(ServerWindow* wnd, int x, int y) {
        dragging_wnd = wnd;
        dragging_initial_window_x = wnd->x();
        dragging_initial_window_y = wnd->y();
        dragging_initial_mouse_x = x;
        dragging_initial_mouse_y = y;
    }
    
    void stop_drag() {
        dragging_wnd = 0;
    }
    
    void invalidate_mouse(Rect rc, bool draw) {
        List<Rect> rcs;
        rcs.append(rc);
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
        bool activated = false;
        if (!wnd->keep_inactive()) {
            activated = activate(wnd);
        }
        if (!activated) {
            wnd->paint();
        }
    }

    void destroy(ServerWindow* wnd) {
        wnds.remove(wnd);
        wnd->destroy();
        List<Rect> rcs;
        rcs.append(wnd->window_rect_in_screen_coord());
        invalidate(rcs);
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
        bool activated = false;
        if (!wnd->keep_inactive()) {
            wnd->activate();
            put_at_top(wnd);
            wnd->paint();
            activated = true;
        }
        return activated;
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
        auto update_rcs(rcs);
        Array<ServerWindow*> wnds_to_draw;
        for (auto wnd: reversed(wnds)) {
            if (!rcs.empty() && wnd->clip(rcs)) {
                wnds_to_draw.append(wnd);
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
    
    ServerWindow* dragging_wnd = 0;
    int dragging_initial_window_x;
    int dragging_initial_window_y;
    int dragging_initial_mouse_x;
    int dragging_initial_mouse_y;
};

int main() {
    auto info = new ScreenInfo;
    get_screen_info(info);
    screen_width = info->width;
    screen_height = info->height;
    screen_bpp = info->bpp;
    screen_pitch = info->pitch;
    video_mem = info->video_mem;
    delete info;
    Server server;
    server.exec();
}
