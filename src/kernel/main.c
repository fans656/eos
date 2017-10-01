#include "def.h"
#include "elf.h"
#include "stdio.h"
#include "memory.h"
#include "interrupt.h"
#include "util.h"
#include "disk.h"
#include "string.h"
#include "assert.h"
#include "filesystem.h"
#include "graphics.h"
#include "math.h"

void load_elf();

void main() {
    init_console();
    init_memory();
    init_filesystem();
    init_graphics();
    init_interrupt();
    
    putchar('a');
    putchar('b');
    putchar('c');
    draw_bmp_at("/img/walle.bmp", 200, 300);
    draw_bmp_at("/img/girl.bmp", 500, 0);
    
    hlt_forever();
}

typedef void (*Func)();

void load_elf() {
    char* name = "/art";
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
}
