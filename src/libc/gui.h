#ifndef GUI_H
#define GUI_H

#include "../kernel/guitypes.h"

Window* wnd_new(WndProc wndproc);
int gui_main(Window* wnd);

#endif
