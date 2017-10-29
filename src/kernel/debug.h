#ifndef _DEBUG_H
#define _DEBUG_H

#include "def.h"
#include "time.h"

struct DebugInfo {
    uint proc_switch_cnt = 0;
    clock_t current_ticks = 0;
    
    void dump();
};

extern DebugInfo debug_info;

#endif
