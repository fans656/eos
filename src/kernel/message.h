#ifndef MESSAGE_H
#define MESSAGE_H

#include "def.h"

extern "C" void* get_message(int id, bool blocking);
extern "C" void put_message(int id, void* message);
void init_message();

#endif
