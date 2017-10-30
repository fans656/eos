#ifndef EOS_H
#define EOS_H

#include "def.h"

void* get_message(int id, bool blocking = true);
void put_message(int id, void* message);
uint set_timer(uint ms, uint id, bool singleshot = false);

void memory_blit(const uchar* buffer, int src_pitch,
        int src_left, int src_top,
        int dst_left, int dst_top,
        int width, int height);

void get_screen_info(ScreenInfo* info);
void execute(const char* fpath);

#endif
