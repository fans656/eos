/*

http://wiki.osdev.org/Memory_Map_(x86)
0x00000500	0x00007BFF	almost 30 KiB	RAM (guaranteed free for use)	        Conventional memory
0x00007C00 	0x00007DFF	512 bytes	    RAM - partially unusable (see above)	Your OS BootSector
0x00007E00	0x0007FFFF	480.5 KiB	    RAM (guaranteed free for use)	        Conventional memory

================================================================

Source          Image       PhyAddr             Disk

boot/mbr.s      mbr.img     [7c00h..7e00h)      sector-[0..1)

    build.sh        dd if=mbr.img

boot/boot.c     boot.img    [7e00..8c00h)       sector-[1..8)

    build.sh        dd seek=1
    boot/mbr.s      LoadBootloader
    boot/mbr.s      jmp (1 << 3):MBREnd

*.c             kernel.img  [100000h..200000h)  [4KB..1MB)

    build.sh        dd seek=8
    filesystem.c    #define RESERVED_BLOCK 256

================================================================

Free memory usage:

0x500       kernel_end - used to setup kernel stack
    
    boot/boot.c     asm("mov [0x500] ...
    entry.c         mov eax, [0x500]; mov esp, eax
    memory.c        uint* kernel_end = ...

0x600       vesa_info - used to get the screen width, height, etc
    
    boot/mbr.asm    SwitchToVesaMode

0x700       mem_map - used to get usable memory
    
    boot/mbr.asm    GetMemoryMap

0x6000      kernel_elf - used to load kernel elf header + program header
    
    boot/boot.c     ELFHeader* elf = ... ; read_segment(elf, 4 * KB...

0x7c00      MBR
0x7e00      2-stage bootloader
0x100000    kernel

================================================================

After paging:

0xc0000000      first 1MB
0xc0100000      kernel

 */
#ifndef DEF_H
#define DEF_H

#define KB 1024U
#define MB (1024 * KB)
#define GB (1024 * MB)

#define KERNEL_PADDR (1 * MB)
#define KERNEL_BASE (3 * GB)
#define VIDEO_MEM 0xb8000

#define SECTOR_SIZE 512
#define STACK_SIZE (64 * KB)

#define PAGE_SIZE 4096

#define V2P(x) ((uint)(x) - KERNEL_BASE)
#define P2V(x) ((uint)(x) + KERNEL_BASE)

#define CHAR(row, col) (*(video_mem + (row) * COLS + (col)))

#define PTE_P 0x001
#define PTE_W 0x002
#define PTE_PS 0x080

enum {
    SYSCALL_PRINTF = 1,
};

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long long ulonglong;
typedef uint size_t;

#define bool uchar
#define true 1
#define false 0

#endif
