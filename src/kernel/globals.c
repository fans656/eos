#include "globals.h"

void init_globals() {
    cursor_cur_row = 0;
    cursor_cur_col = 0;
    num_key_pressed = 0;
    for (int i = 0; i < 128; ++i) {
        key_states[i] = 0;
    }
}
