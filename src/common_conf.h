#include "types.h"

#define KB 1024U
#define MB (1024 * KB)
#define GB (1024 * MB)

#define ENTRY_INFO_ADDR (0x500)
#define ENTRY_INFO_KERNEL_END (ENTRY_INFO_ADDR)
#define ENTRY_INFO_VESA_INFO ((ENTRY_INFO_ADDR) + 4)
// http://www.delorie.com/djgpp/doc/rbinter/it/79/0.html
#define ENTRY_INFO_END ((ENTRY_INFO_VESA_INFO) + 256)

#define KERNEL_ELF_ADDR (ENTRY_INFO_ADDR + ENTRY_INFO_END)

#define KERNEL_PADDR (1 * MB)
#define KERNEL_BASE (3 * GB)
#define VIDEO_MEM 0xb8000

#define SECTOR_SIZE 512
#define STACK_SIZE 4096

#define PHY_MEM 120  // MB, must be a mutiple of 4 (maximum less than 1GB)
