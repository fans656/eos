#ifndef MOUSE_H
#define MOUSE_H

#include "def.h"

extern int mouse_x, mouse_y;

void init_mouse(int screen_width, int screen_height);
void parse_mouse_event(uchar* b, GUIMouseEvent& ev);

#endif
