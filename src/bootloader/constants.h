#ifndef CONSTANTS_H
#define CONSTANTS_H

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

#define HEAP_BEG (128 * KB)
#define HEAP_END (HEAP_BEG + 128 * KB)
#define STACK_BEG HEAP_END
#define STACK_END (STACK_BEG + 64 * KB)

#define KERNEL_BEG_PADDR (1 * MB)
#define KERNEL_BEG_VADDR 0xc0100000

#define MEMORY_META_OFFSET (4 * MB - 12 * KB - 1 * KB)

#define MAX_KERNEL_SIZE (MEMORY_META_OFFSET)

#define FRAMES_OFFSET (4 * MB - 12 * KB)
#define PAGE_DIRECTORY_OFFSET (FRAMES_OFFSET + 4 * MB)
#define PAGE_TABLE_BASE_OFFSET (PAGE_DIRECTORY_OFFSET + 3 * 4 * KB)

#define PAGE_TABLES_MAX_SIZE (4 * MB)

#define KERNEL_STACK_SIZE (4 * MB)
#define KERNEL_STACK_BEG_VADDR (0xffffffff - KERNEL_STACK_SIZE + 1)
#define KERNEL_STACK_BEG_PADDR (KERNEL_BEG_PADDR + PAGE_TABLE_BASE_OFFSET + PAGE_TABLES_MAX_SIZE)
#define KERNEL_STACK_END_PADDR (KERNEL_STACK_BEG_PADDR + KERNEL_STACK_SIZE)

#define FRAMES_BEG_PADDR (KERNEL_BEG_PADDR + FRAMES_OFFSET)
#define FRAMES_END_PADDR (FRAMES_BEG_PADDR + 4 * MB)

#define MEMORY_META_PADDR (KERNEL_BEG_PADDR + MEMORY_META_OFFSET)
#define MEMORY_META_VADDR (KERNEL_BEG_VADDR + MEMORY_META_OFFSET)

#define PAGE_DIRECTORY_PADDR (KERNEL_BEG_PADDR + PAGE_DIRECTORY_OFFSET)
#define PAGE_DIRECTORY_VADDR (KERNEL_BEG_VADDR + PAGE_DIRECTORY_OFFSET)
#define PAGE_TABLE_BASE_PADDR (KERNEL_BEG_PADDR + PAGE_TABLE_BASE_OFFSET)
#define PAGE_TABLE_BASE_VADDR (KERNEL_BEG_VADDR + PAGE_TABLE_BASE_OFFSET)

enum {
    SYSCALL_PRINTF = 1,
};

#endif
