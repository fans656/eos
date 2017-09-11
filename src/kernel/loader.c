/*
https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
 */
#include <stdint.h>
#include "loader.h"
#include "malloc.h"
#include "filesystem.h"
#include "io.h"
#include "util.h"

typedef struct __attribute__((packed)) {
    char magic[4];
    uint8_t arch;
    uint8_t endianness;
    uint8_t version;
    uint8_t os_abi;
    uint8_t abi_version;
    uint8_t unused[7];
    uint16_t type;
    uint16_t machine;
    uint32_t version_;
    char* entry;
    uint32_t ph_offset;
    uint32_t sh_offset;
    uint32_t flags;
    uint16_t header_size;
    uint16_t ph_size;
    uint16_t ph_entries;
    uint16_t sh_size;
    uint16_t sh_entries;
    uint16_t i_section_names;
} FileHeader;

typedef struct __attribute__((packed)) {
    uint32_t type;
    uint32_t offset;
    char* vaddr;
    uint32_t paddr;
    uint32_t file_size;
    uint32_t mem_size;
    uint32_t flags;
    uint32_t align;
} ProgramHeader;

typedef struct __attribute__((packed)) {
    uint32_t name_offset;
    uint32_t type;
    uint32_t flags;
    char* vaddr;
    uint32_t file_offset;
    uint32_t file_size;
    uint32_t link;
    uint32_t info;
    uint32_t align;
    uint32_t entry_size;
} SectionHeader;

void execute(char* fpath) {
    FILE* fp = fopen(fpath);
    char* content = malloc(fsize(fp));
    fread(fp, fsize(fp), content);

    FileHeader* fh = (FileHeader*)content;
    
    SectionHeader* name_section = (SectionHeader*)(content + fh->sh_offset) + fh->i_section_names;
    char* name_content = content + name_section->file_offset;

    uint32_t addr_min = (uint32_t)fh->entry;
    uint32_t addr_max = (uint32_t)fh->entry + 0x2000000;

    SectionHeader* sh = (SectionHeader*)(content + fh->sh_offset);
    for (int i = 0; i < fh->sh_entries; ++i) {
        uint32_t addr_beg = (uint32_t)sh->vaddr;
        uint32_t addr_end = (uint32_t)sh->vaddr + sh->file_size;
        char* name = name_content + sh->name_offset;
        if (addr_min <= addr_beg && addr_beg < addr_end && addr_end <= addr_max) {
            memcpy(content + sh->file_offset, sh->vaddr, sh->file_size);
            if (strcmp(name, ".bss") == 0) {
                memset(sh->vaddr, sh->file_size, 0);
            }
        }
        ++sh;
    }
    asm volatile("call %0" :: "g"((uint32_t)fh->entry));
}
