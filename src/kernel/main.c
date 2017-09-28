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

typedef struct __attribute__((__packed__)) {
    uchar magic[2];
    uchar mode;
    uchar charsize;
} PSF1;

void main() {
    init_console();
    init_memory();
    init_interrupt();
    init_filesystem();
    init_graphics();
    
    char* name = "/img/walle.bmp";
    size_t size = fsize(name);
    char* buffer = malloc(size);
    FILE* fp = fopen(name);
    fread(fp, size, buffer);
    fclose(fp);
    
    printf("%s\n\n", name);
    hexdump(buffer, 128);
    sync_console();

    draw_bmp_at(name, 300, 200);
    
    hlt_forever();
}

typedef void (*Func)();

void load_elf() {
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
}
