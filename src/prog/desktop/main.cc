#include "stdio.h"
#include "stdlib.h"
#include "eos.h"
#include "gui.h"
#include "time.h"

constexpr int ICON_MARGIN = 20;
constexpr int ICON_MARGIN_BORDER = 8;
constexpr int N_ICONS = 4;

struct Wnd : public Window {
    Wnd(int width, int height, uint attr)
        : Window(0, 0, width, height, attr) {
        background = new Bitmap("/img/cheetah.png");
        icons[0] = new Bitmap("/img/icon-pa.png");
        icons[1] = new Bitmap("/img/icon-pb.png");
        icons[2] = new Bitmap("/img/icon-pc.png");
        icons[3] = new Bitmap("/img/icon-terminal.png");
        fpaths[0] = "/bin/pa";
        fpaths[1] = "/bin/pb";
        fpaths[2] = "/bin/pc";
        fpaths[3] = "/bin/terminal";
    }
    
    ~Wnd() {
        delete background;
        for (int i = 0; i < N_ICONS; ++i) {
            delete icons[i];
        }
    }
    
    void on_paint(PaintEvent* ev) {
        Painter painter(this);
        painter.draw_bitmap(0, 0, background);
        int side = icons[0]->height();
        for (int i = 0; i < N_ICONS; ++i) {
            if (i == i_selected_icon) {
                painter.fill_rect(Rect(
                            ICON_MARGIN - ICON_MARGIN_BORDER, 
                            ICON_MARGIN - ICON_MARGIN_BORDER
                            + (icons[i]->height() + ICON_MARGIN) * i,
                            side + 2 * ICON_MARGIN_BORDER,
                            side + 2 * ICON_MARGIN_BORDER), SteelBlue);
            }
            painter.draw_bitmap(
                    ICON_MARGIN,
                    ICON_MARGIN + (icons[i]->height() + ICON_MARGIN) * i,
                    icons[i]);
        }
    }
    
    void on_mouse(MouseEvent* ev) {
        int x = ev->x;
        int y = ev->y;
        int side = icons[0]->height();
        int index = -1;
        if (ICON_MARGIN <= x && x < ICON_MARGIN + side) {
            if (y >= ICON_MARGIN) {
                y -= ICON_MARGIN;
                int i = y / (side + ICON_MARGIN);
                if (i < N_ICONS && y % (side + ICON_MARGIN) < side) {
                    index = i;
                }
            }
        }
        if (ev->buttons & 1) {
            if (i_selected_icon != index) {
                i_selected_icon = index;
                update();
            } else if (i_selected_icon >= 0) {
                execute(fpaths[i_selected_icon]);
                i_selected_icon = -1;
                update();
            }
        }
    }
    
    Bitmap* background;
    Bitmap* icons[N_ICONS];
    const char* fpaths[N_ICONS];
    int i_selected_icon = -1;
};

int main() {
    //while (true) {
    //    for (int i = 0; i < 10000; ++i) {
    //        for (int j = 0; j < 1000; ++j) {
    //        }
    //    }
    //    put_message(QUEUE_ID_GUI, new int);
    //}
    auto info = new ScreenInfo;
    get_screen_info(info);
    Wnd* wnd = new Wnd(info->width, info->height,
            WND_CLIENT_ONLY | WND_KEEP_INACTIVE | WND_KEEP_BOTTOM
            | WND_NO_CLIENT_FILL);
    delete info;
    wnd->exec();
    delete wnd;
}
