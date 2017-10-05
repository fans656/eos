#ifndef PROCESS_H
#define PROCESS_H

#include "def.h"

typedef struct _Process {
    uint* pgdir;  // pgdir & esp must be first two and order is important
    uint esp;     // they're used in isr_timer.asm

    const char* path;
    uint pid;
    uint entry;
} _Process;
typedef _Process* Process;

extern uint current_esp;
extern Process running_proc;

extern "C" uint process_schedule();
void process_exit(int status);
void process_release();

void process_wake_event_waiting(Process target);
void process_make_event_waiting();
void process_sleep(uint ms);
void process_count_down();

void init_process();

#endif
