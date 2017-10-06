#ifndef EOS_H
#define EOS_H

#include "def.h"

void* get_message(int id, bool blocking);
void put_message(int id, void* message);

#endif
