#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include <stdbool.h>
#include "util.h"

extern uint8_t cursor_cur_row;
extern uint8_t cursor_cur_col;
extern int num_key_pressed;
extern bool key_states[];

void init_globals();

#endif
