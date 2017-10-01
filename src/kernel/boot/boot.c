/*
https://docs.oracle.com/cd/E19455-01/806-3773/6jct9o0aj/index.html
https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html#Machine-Constraints
*/
#include "../def.h"

typedef struct ELFHeader {
    uint magic;
    uchar ignored[12];
    ushort type;
    ushort machine;
    uint version;
    uint entry;
    uint phoff;
    uint shoff;
    uint flags;
    ushort ehsize;
    ushort phentsize;
    ushort phnum;
    ushort shentsize;
    ushort shnum;
    ushort shstrndx;
} ELFHeader;

typedef struct ProgramHeader {
    uint type;
    uint offset;
    uint vaddr;
    uint paddr;
    uint filesz;
    uint memsz;
    uint flags;
    uint align;
} ProgramHeader;

uchar inb(ushort port);
void insd(int port, void* addr, int cnt);
void outb(ushort port, uchar val);
void stosb(void* addr, uint count, uchar val);

void hexdump(char* addr);
void read_sector(void* addr, uint offset);
void read_segment(void* addr, uint count, uint offset);

void bootmain() {
    ELFHeader* elf = (ELFHeader*)0x6000;
    read_segment(elf, 4 * KB, 0);

    ProgramHeader* ph = (ProgramHeader*)((uchar*)elf + elf->phoff);
    ProgramHeader* eph = ph + elf->phnum;
    char* end = 0;
    for (; ph < eph; ph++) {
        if (ph->type == 1 && ph->vaddr >= KERNEL_BASE) {
            char* addr = (char*)(ph->vaddr - KERNEL_BASE);
            read_segment(addr, ph->filesz, ph->offset);
            if (ph->memsz > ph->filesz) {
                stosb(addr + ph->filesz, ph->memsz - ph->filesz, 0);
            }
            if ((char*)(ph->vaddr + ph->memsz) > end) {
                end = (char*)(ph->vaddr + ph->memsz);
            }
        }
    }
    end += STACK_SIZE;
    end += PAGE_SIZE - (uint)end % PAGE_SIZE;
    asm volatile("mov [0x500], %0" ::  "a"(end));
    
    asm volatile("jmp %0" :: "r"(elf->entry - KERNEL_BASE));
}

#define HEX(c) ((c) <= 9 ? ((c) + '0') : ((c) - 10 + 'A'))

void hexdump(char* addr) {
    ushort* video = (ushort*)VIDEO_MEM;
    for (int i = 0; i < 8; ++i) {
        uchar val = *addr++;
        uchar ch_high = HEX(val >> 4);
        uchar ch_low = HEX(val & 0x0f);
        video[i * 3] = 0x0f00 | ch_high;
        video[i * 3 + 1] = 0x0f00 | ch_low;
        video[i * 3 + 2] = 0x0f00 | ' ';
    }
    asm("hlt");
}

void wait_disk() {
    while ((inb(0x1f7) & 0xc0) != 0x40) {
        ;
    }
}

void read_sector(void* addr, uint offset) {
    wait_disk();
    outb(0x1f2, 1);
    outb(0x1f3, offset);
    outb(0x1f4, offset >> 8);
    outb(0x1f5, offset >> 16);
    outb(0x1f6, (offset >> 24) | 0xe0);
    outb(0x1f7, 0x20);
    wait_disk();
    insd(0x1f0, addr, SECTOR_SIZE / 4);
}

void read_segment(void* addr, uint count, uint offset) {
    void* end_addr = addr + count;
    addr -= offset % SECTOR_SIZE;
    offset = offset / SECTOR_SIZE + 8;
    for (; addr < end_addr; addr += SECTOR_SIZE, offset++) {
        read_sector(addr, offset);
    }
}

void insd(int port, void* addr, int cnt) {
    asm volatile("cld; rep insd"
            : "=D"(addr), "=c"(cnt)
            : "d"(port), "0"(addr), "1"(cnt)
            : "memory", "cc");
}

void stosb(void* addr, uint count, uchar val) {
    asm volatile("cld; rep stosb"
            : "=D"(addr), "=c"(count)
            : "0"(addr), "1"(count), "a"(val)
            : "memory", "cc");
}

uchar inb(ushort port) {
    uchar res;
    asm volatile("inb %0, %1" : "=a"(res) : "d"(port));
    return res;
}

void outb(ushort port, uchar val) {
    asm volatile("outb %0, %1" :: "d"(port), "a"(val));
}
