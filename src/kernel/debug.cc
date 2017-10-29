#include "debug.h"
#include "stdio.h"
#include "memory.h"

DebugInfo debug_info;

uint debug_cnt = 0;

void DebugInfo::dump() {
    debug("=============================================\n");
    debug("current_ticks: %d\n", current_ticks);
    debug("proc_switch_cnt: %d\n", proc_switch_cnt);
    debug("malloc_list_size: %d\n", malloc_list_size());
}
