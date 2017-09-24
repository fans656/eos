#define KB 1024
#define MB (1024 * KB)
#define GB (1024 * MB)

#define KERNEL_BASE 0xc0000000

#define VIDEO_MEM (0xb8000 + KERNEL_BASE)

#define PAGE_SIZE 4096

#define PTE_P 0x001
#define PTE_W 0x002
#define PTE_PS 0x080
