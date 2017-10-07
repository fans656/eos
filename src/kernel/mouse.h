#ifndef MOUSE_H
#define MOUSE_H

#include "def.h"

extern int mouse_x, mouse_y;

void parse_mouse_event(uchar* b, GUIMouseEvent& ev);

#endif
