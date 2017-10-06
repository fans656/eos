#include "app.h"
#include "eos.h"
#include "list.h"
#include "stdio.h"

enum Type {
    CreateWindow,
};

struct Message {
    Type type;
};

struct CreateWindowMessage : public Message {
    CreateWindowMessage(Window* wnd) {
        type = CreateWindow;
        this->wnd = wnd;
    }
    Window* wnd;
};

struct Server {
    void create_window(Window* wnd) {
        top_wnds.append(wnd);
        printf("Server::create_window(%x)\n", wnd);
    }
    
    List<Window*> top_wnds;
};

void gui(Window* wnd) {
    put_message(GUI_MESSAGE_ID, new CreateWindowMessage(wnd));
}

void gui_server() {
    Server server;
    while (true) {
        Message* msg = (Message*)get_message(GUI_MESSAGE_ID);
        if (msg) {
            switch (msg->type) {
                case CreateWindow:
                    server.create_window(((CreateWindowMessage*)msg)->wnd);
                    break;
                default:
                    break;
            }
        }
    }
}
