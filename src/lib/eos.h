#ifndef EOS_H
#define EOS_H

#include "def.h"

void* get_message(int id, bool blocking = true);
void put_message(int id, void* message);

void memory_blit(const uchar* buffer, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height);

#endif
