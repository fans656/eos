#include "elf.h"
#include "filesystem.h"
#include "math.h"
#include "memory.h"
#include "string.h"

ProgEntry load_elf(char* fpath) {
    FILE* fp = fopen(fpath);
    size_t sz = min(4096, fsize(fp));
    ELFHeader* elf = (ELFHeader*)named_malloc(sz, "ELFHeader");

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
    ProgEntry entry = (ProgEntry)elf->entry;
    free(elf);
    return entry;
}
