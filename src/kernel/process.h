#ifndef PROCESS_H
#define PROCESS_H

#include "def.h"

extern const char* str_changed;

uint process_schedule();
void process_exit(int status);

void init_process();

#endif
