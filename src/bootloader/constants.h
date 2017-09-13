#ifndef CONSTANTS_H
#define CONSTANTS_H

#define RAND_MAX 65536

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

/*
Memory layout:
    0-1MB       kernel, IDT
    1-8MB       page table
    8-16MB      stack
    16-32MB     heap
    3.95GB      graphic video memory
 */
#define KERNEL_END (1 * MB)
#define STACK_END (16 * MB)
#define HEAP_BEG STACK_END
#define HEAP_END (32 * MB)

enum {
    SYSCALL_PRINTF = 1,
};

#endif
