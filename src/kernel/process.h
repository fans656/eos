#ifndef PROCESS_H
#define PROCESS_H

#include "def.h"

void proc_new(const char* path);

uint process_schedule();

void init_process();

void process_exit(int status);

#endif
