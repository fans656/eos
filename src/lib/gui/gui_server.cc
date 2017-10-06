#include "gui_server.h"
#include "gui_message.h"
#include "window.h"
#include "eos.h"
#include "list.h"
#include "stdio.h"
#include "graphics.h"

struct Server {
    void create_window(Window* wnd) {
        top_wnds.append(wnd);
        put_message((int)wnd->app, new CreateEvent(wnd));
        put_message((int)wnd->app, new PaintEvent(wnd));
    }
    
    void on_window_painted(Window* wnd) {
        char* bmp = (char*)load_file("/img/girl.bmp");
        memory_blit(bmp_data(bmp), bmp_pitch(bmp), 0, 0, 400, 0, bmp_width(bmp), bmp_height(bmp));
    }
    
    List<Window*> top_wnds;
};

void gui_server() {
    Server server;
    while (true) {
        Message* msg = (Message*)get_message(GUI_MESSAGE_ID);
        if (msg) {
            switch (msg->type) {
                case CreateWindow:
                    {
                        printf("server got CreateWindow\n");
                        CreateWindowMessage* msg_ = (CreateWindowMessage*)msg;
                        server.create_window(msg_->wnd);
                        break;
                    }
                case WindowPainted:
                    {
                        printf("server got WindowPainted\n");
                        WindowPaintedMessage* msg_ = (WindowPaintedMessage*)msg;
                        server.on_window_painted(msg_->wnd);
                        break;
                    }
                default:
                    break;
            }
        }
    }
}
