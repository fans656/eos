#ifndef EOS_H
#define EOS_H

#include "def.h"

enum {
    GUI_MESSAGE_ID,
};

void* get_message(int id, bool blocking = true);
void put_message(int id, void* message);

#endif
