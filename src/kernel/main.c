#include "stdio.h"
#include "memory.h"
#include "interrupt.h"
#include "util.h"
#include "disk.h"
#include "elf.h"
#include "string.h"
#include "assert.h"
#include "filesystem.h"
#include "graphics.h"
#include "math.h"

typedef void (*Func)();

void main() {
    init_console();
    init_memory();
    init_interrupt();
    init_filesystem();
    
    char* name = "/hello";
    size_t sz = min(4096, fsize(name));
    ELFHeader* elf = (ELFHeader*)malloc(sz);

    FILE* fp = fopen(name);
    fread(fp, sz, elf);

    ProgramHeader* ph = (ProgramHeader*)((char*)elf + elf->phoff);
    for (int i = 0; i < elf->phnum; ++i, ++ph) { 
        if (ph->type == 1) {
            uint vbeg = ph->vaddr;
            uint vend = vbeg + ph->memsz;
            map_pages(pgdir, vbeg, ph->memsz);
            fseek(fp, ph->offset);
            fread(fp, ph->filesz, (void*)ph->vaddr);
            if (ph->memsz > ph->filesz) {
                memset((uchar*)ph->vaddr + ph->filesz, 0, ph->memsz - ph->filesz);
            }
        }
    }
    ((Func)elf->entry)();
    free(elf);
    
    hlt_forever();
}

typedef void (*Func)();
