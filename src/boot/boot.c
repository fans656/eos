/*
I/O instructions
https://docs.oracle.com/cd/E19455-01/806-3773/6jct9o0aj/index.html

Inline Assembly
https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html
https://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html#Machine-Constraints

Constraints:
    "c" - ecx
    "d" - edx
    "D" - edi

Clobbers:
    "memory" - will change memory
    "cc" - will change FLAGS register
*/
#include <stdint.h>

#define KERNEL_PADDR 0x100000
#define KERNEL_BASE 0xc0000000
#define VIDEO_MEM 0xb8000

#define SECTOR_SIZE 512
#define STACK_SIZE 4096

#define HEX(c) ((c) <= 9 ? ((c) + '0') : ((c) - 10 + 'A'))

typedef struct {
    uint32_t magic;
    uint8_t ignored[12];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} ELFHeader;

typedef struct {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} ProgramHeader;

uint8_t inb(uint16_t port);
void insl(int port, void* addr, int cnt);
void outb(uint16_t port, uint8_t val);
void stosb(void* addr, uint32_t count, uint8_t val);

void hexdump(char* addr);
void read_sector(void* addr, uint32_t offset);
void read_segment(void* addr, uint32_t count, uint32_t offset);

void bootmain() {
    ELFHeader* elf = (ELFHeader*)(0x500 + 4096);
    read_segment(elf, 4096, 0);

    ProgramHeader* ph = (ProgramHeader*)((uint8_t*)elf + elf->phoff);
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
    end += 4096 - (uint32_t)end % 4096;
    asm volatile("movl %0, 0x500" :: "r"(end));
    
    asm volatile("jmp *%0" :: "r"(elf->entry - KERNEL_BASE));
}

void hexdump(char* addr) {
    uint16_t* video = (uint16_t*)VIDEO_MEM;
    for (int i = 0; i < 8; ++i) {
        uint8_t val = *addr++;
        uint8_t ch_high = HEX(val >> 4);
        uint8_t ch_low = HEX(val & 0x0f);
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

void read_sector(void* addr, uint32_t offset) {
    wait_disk();
    outb(0x1f2, 1);
    outb(0x1f3, offset);
    outb(0x1f4, offset >> 8);
    outb(0x1f5, offset >> 16);
    outb(0x1f6, (offset >> 24) | 0xe0);
    outb(0x1f7, 0x20);
    wait_disk();
    insl(0x1f0, addr, SECTOR_SIZE / 4);
}

void read_segment(void* addr, uint32_t count, uint32_t offset) {
    void* end_addr = addr + count;
    addr -= offset % SECTOR_SIZE;
    offset = offset / SECTOR_SIZE + 8;
    for (; addr < end_addr; addr += SECTOR_SIZE, offset++) {
        read_sector(addr, offset);
    }
}

uint8_t inb(uint16_t port) {
    uint8_t res;
    asm volatile("in %1, %0" : "=a"(res) : "d"(port));
    return res;
}

void insl(int port, void* addr, int cnt) {
    asm volatile("cld; rep insl"
            : "=D"(addr), "=c"(cnt)
            : "d"(port), "0"(addr), "1"(cnt)
            : "memory", "cc");
}

void outb(uint16_t port, uint8_t val) {
    asm volatile("out %0, %1" :: "a"(val), "d"(port));
}

void stosb(void* addr, uint32_t count, uint8_t val) {
    asm volatile("cld; rep stosb"
            : "=D"(addr), "=c"(count)
            : "0"(addr), "1"(count), "a"(val)
            : "memory", "cc");
}
