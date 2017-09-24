#include "stdio.h"
#include "memory.h"
#include "interrupt.h"
#include "util.h"
#include "disk.h"
#include "elf.h"
#include "string.h"

typedef void (*Func)();

void main() {
    init_console();
    init_memory();
    init_interrupt();
    
    char buffer[512];
    uint hello_base = 1 * MB;
    read_bytes(hello_base, 512, buffer);
    ELFHeader* elf = (ELFHeader*)buffer;
    ProgramHeader* ph = (ProgramHeader*)((char*)elf + elf->phoff);
    for (int i = 0; i < elf->phnum; ++i, ++ph) { 
        if (ph->type == 1) {
            uint vbeg = ph->vaddr;
            uint vend = vbeg + ph->memsz;
            map_pages(pgdir, vbeg, ph->memsz);
            read_bytes(hello_base + ph->offset, ph->filesz, (void*)ph->vaddr);
            if (ph->memsz > ph->filesz) {
                memset((uchar*)ph->vaddr + ph->filesz, 0, ph->memsz - ph->filesz);
            }
        }
    }
    ((Func)elf->entry)();
    
    hlt_forever();
}
