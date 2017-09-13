#include "io.h"
#include "util.h"
#include "filesystem.h"
#include "malloc.h"
#include "constants.h"
#include "loader.h"

void main();

void kernel_entry() {
    asm volatile (
            "mov ax, 16;"
            "mov ds, ax;"
            "mov es, ax;"
            "mov ss, ax;"
            "mov ebp, 0;"
            "mov esp, %0;" :: "i"(STACK_END)
            );
    clear_screen();
    init_filesystem();
    main();
    hlt();
}

void main() {
    FILE* fp = fopen("/kernel/kernel.img");
    uint32_t kernel_size = fsize(fp);
    char* content = malloc(kernel_size);
    fread(fp, kernel_size, content);

    FileHeader* fh = (FileHeader*)content;
    
    SectionHeader* name_section = (SectionHeader*)(content + fh->sh_offset) + fh->i_section_names;
    char* name_content = content + name_section->file_offset;

    uint32_t addr_min = (uint32_t)fh->entry;
    uint32_t addr_max = (uint32_t)fh->entry + 0x2000000;

    SectionHeader* sh = (SectionHeader*)(content + fh->sh_offset);
    uint32_t kernel_vaddr = 0xc0000000;
    uint32_t base = 32 * MB - kernel_vaddr;
    for (int i = 0; i < fh->sh_entries; ++i) {
        uint32_t addr_beg = (uint32_t)sh->vaddr;
        uint32_t addr_end = (uint32_t)sh->vaddr + sh->file_size;
        char* name = name_content + sh->name_offset;
        if (addr_min <= addr_beg && addr_beg < addr_end && addr_end <= addr_max) {
            memcpy(content + sh->file_offset, sh->vaddr + base, sh->file_size);
            if (strcmp(name, ".bss") == 0) {
                memset(sh->vaddr + base, sh->file_size, 0);
            }
        }
        ++sh;
    }
    
    int kernel_pages = (kernel_size + 32 * MB + 4 * KB - 1) / (4 * KB);
    for (int i = 0; i < 1024; ++i) {
        uint32_t address = ((uint32_t)(page_table) + i * 4 * KB) | 3;
        page_directory[i] = address;
    }
    for (int i_table = 0; i_table < 1024; ++i_table) {
        for (int i_entry = 0; i_entry < 1024; ++i_entry) {
            uint32_t address = i_table * 4 * MB + i_entry * 4 * KB;
            *(uint32_t*)((uint32_t)page_table + i_table * 4 * KB + i_entry * 4) = address | 3;
        }
    }
    for (int i = 0; i < kernel_pages; ++i) {
        uint32_t addr = 32 * MB + i * 4 * KB;
        *(uint32_t*)((uint32_t)page_table + 256 * 3 * 4 * KB + i * 4) = addr | 3;
    }
    asm volatile(
            "mov eax, %0;"
            "mov cr3, eax;"
            "mov eax, cr0;"
            "or eax, 0x80000001;"
            "mov cr0, eax;"
            :: "g"((uint32_t)page_directory)
            );

    print_mem((void*)(32 * MB), 128);
    printf("\n");
    print_mem((void*)(kernel_vaddr), 128);
    
    asm("jmp 0xc0000000");
}
