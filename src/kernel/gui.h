#ifndef GUI_H
#define GUI_H

#include "guitypes.h"

int register_window(Window* wnd);
Event* get_event(Window* wnd);
void init_gui();
void gui_work();

#endif
