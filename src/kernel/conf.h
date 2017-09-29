#ifndef CONF_H
#define CONF_H

#include "../types.h"
#include "../common_conf.h"

#define V2P(x) ((uint)(x) - KERNEL_BASE)
#define P2V(x) ((uint)(x) + KERNEL_BASE)

#define CHAR(row, col) (*(video_mem + (row) * COLS + (col)))

#define PAGE_SIZE 4096

#define PTE_P 0x001
#define PTE_W 0x002
#define PTE_PS 0x080

enum {
    SYSCALL_PRINTF = 1,
};

#endif
