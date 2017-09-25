#include "stdio.h"
#include "memory.h"
#include "interrupt.h"
#include "util.h"
#include "disk.h"
#include "elf.h"
#include "string.h"
#include "assert.h"

typedef void (*Func)();

void main() {
    init_console();
    init_memory(128 * MB);
    init_interrupt();

    char* buffer = malloc(4096);
    uint hello_file_offset = 1 * MB;
    read_bytes(hello_file_offset, 4096, buffer);
    ELFHeader* elf = (ELFHeader*)buffer;
    ProgramHeader* ph = (ProgramHeader*)((char*)elf + elf->phoff);
    for (int i = 0; i < elf->phnum; ++i, ++ph) { 
        if (ph->type == 1) {
            uint vbeg = ph->vaddr;
            uint vend = vbeg + ph->memsz;
            map_pages(pgdir, vbeg, ph->memsz);
            read_bytes(hello_file_offset + ph->offset, ph->filesz, (void*)ph->vaddr);
            if (ph->memsz > ph->filesz) {
                memset((uchar*)ph->vaddr + ph->filesz, 0, ph->memsz - ph->filesz);
            }
        }
    }
    ((Func)elf->entry)();
    free(buffer);
    
    hlt_forever();
}

void load_elf() {
}
