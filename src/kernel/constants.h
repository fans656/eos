#ifndef CONSTANTS_H
#define CONSTANTS_H

#define RAND_MAX 65536

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

#define VADDR_BASE 0xc0000000

#define KERNEL_BEG (0xc0100000)
#define FRAMES_ADDR (KERNEL_BEG + 4 * MB - 12 * KB)
#define MEMORY_META_ADDR (FRAMES_ADDR - 1 * KB)
#define PAGE_DIRECTORY_ADDR (FRAMES_ADDR + 4 * MB)
#define HEAP_BEG (KERNEL_BEG + 12 * MB)

#define STACK_BEG (0xffffffff - 4 * MB + 1)
#define STACK_END (0xffffffff - 4 + 1)

enum {
    SYSCALL_PRINTF = 1,
};

#endif
