#ifndef GUI_APP_H
#define GUI_APP_H

#include "list.h"

struct Window;

struct App {
    void exec();
    void add(Window* wnd);
    bool finished() { return false; }
    
    List<Window*> wnds;
};

void gui(Window* wnd);

#endif
