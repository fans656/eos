#ifndef LOADER_H
#define LOADER_H

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
    
#endif
