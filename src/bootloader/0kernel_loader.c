#include "io.h"
#include "util.h"
#include "filesystem.h"
#include "malloc.h"
#include "constants.h"
#include "loader.h"
#include "math.h"

typedef struct {
    uint32_t page_tables_p[1024];
    uint32_t page_tables_v[1024];
    uint32_t paddr;
} PageDirectory;

typedef struct {
    uint32_t* frames_top;
} MemoryMeta;

MemoryMeta* memory_meta = (MemoryMeta*)MEMORY_META_PADDR;

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

uint32_t load_kernel_at() {
    // open kernel image file
    FILE* fp = fopen("/kernel/kernel.img");
    uint32_t kernel_size = fsize(fp);
    assert(kernel_size <= MAX_KERNEL_SIZE);

    // load kernel ELF image into memory
    char* content = (char*)(KERNEL_BEG_PADDR + kernel_size);
    fread(fp, kernel_size, content);
    
    // parse ELF image and put sections to KERNEL_BEG_PADDR start memory
    FileHeader* fh = (FileHeader*)content;
    
    SectionHeader* name_section = (SectionHeader*)(content + fh->sh_offset) + fh->i_section_names;
    char* name_content = content + name_section->file_offset;

    uint32_t addr_min = (uint32_t)fh->entry;
    uint32_t addr_max = (uint32_t)fh->entry + MAX_KERNEL_SIZE;

    SectionHeader* sh = (SectionHeader*)(content + fh->sh_offset);
    for (int i = 0; i < fh->sh_entries; ++i) {
        uint32_t addr_beg = (uint32_t)sh->vaddr;
        uint32_t addr_end = (uint32_t)sh->vaddr + sh->file_size;
        char* name = name_content + sh->name_offset;
        if (addr_min <= addr_beg && addr_beg < addr_end && addr_end <= addr_max) {
            void* paddr = KERNEL_BEG_PADDR + sh->vaddr - KERNEL_BEG_VADDR;
            memcpy(content + sh->file_offset, paddr, sh->file_size);
            if (strcmp(name, ".bss") == 0) {
                memset(paddr, sh->file_size, 0);
            }
        }
        ++sh;
    }
    return kernel_size;
}

uint32_t* init_frames_stack() {
    uint32_t* top = (uint32_t*)(KERNEL_BEG_PADDR + FRAMES_OFFSET + 4 * MB);
    for (uint32_t addr = 0xffffffff - 4 * KB + 1; addr >= KERNEL_STACK_END_PADDR; addr -= 4 * KB) {
        *--top = addr;
    }
    for (int addr = 1 * MB - 4 * KB; addr >= 0; addr -= 4 * KB) {
        *--top = addr;
    }
    memory_meta->frames_top = (uint32_t*)(KERNEL_BEG_VADDR + (uint32_t)top - FRAMES_END_PADDR);
}

void init_page_directory() {
    PageDirectory* page_directory = (PageDirectory*)PAGE_DIRECTORY_PADDR;
    page_directory->paddr = PAGE_DIRECTORY_PADDR;
    for (int i = 0; i < 1024; ++i) {
        (page_directory->page_tables_p)[i] = (page_directory->page_tables_v)[i] = 0;
    }
}

void map_pages(uint32_t vaddr, uint32_t paddr, uint32_t size) {
    PageDirectory* page_directory = (PageDirectory*)PAGE_DIRECTORY_PADDR;
    int i_pde = (vaddr & 0xffc00000) >> 22;
    int i_pte = (vaddr & 0x003ff000) >> 12;
    uint32_t* pte = (uint32_t*)(PAGE_TABLE_BASE_PADDR + i_pde * 4 * KB + i_pte * 4);
    while (size > 0) {
        if (!(page_directory->page_tables_p)[i_pde]) {
            (page_directory->page_tables_p)[i_pde] = (PAGE_TABLE_BASE_PADDR + i_pde * 4 * KB) | 3;
            (page_directory->page_tables_v)[i_pde] = PAGE_TABLE_BASE_VADDR + i_pde * 4 * KB;
            uint32_t* page_table = (uint32_t*)(page_directory->page_tables_p)[i_pde];
            for (int i = 0; i < 1024; ++i) {
                page_table[i] = 0;
            }
        }
        for (int i = 0; i < 1024 && size > 0; ++i) {
            *pte++ = paddr | 3;
            paddr += 4 * KB;
            size -= 4 * KB;
        }
        ++i_pde;
    }
}

void main() {
    load_kernel_at(KERNEL_BEG_PADDR);
    
    //init_frames_stack();

    init_page_directory();
    
    //uint32_t paddr = 0;
    //for (int i = 256 * 3; i < 256 * 3 + 4; ++i) {
    //    *(uint32_t*)(PAGE_DIRECTORY_PADDR + i * 4)
    //        = (PAGE_DIRECTORY_PADDR + 12 * KB + i * 4 * KB) | 3;
    //    for (int j = 0; j < 1024; ++j) {
    //        *(uint32_t*)(PAGE_DIRECTORY_PADDR + 12 * KB + i * 4 * KB + j * 4)
    //            = paddr | 3;
    //        paddr += 4 * KB;
    //    }
    //}
    //
    //*(uint32_t*)(PAGE_DIRECTORY_PADDR) = (PAGE_DIRECTORY_PADDR + 12 * KB) | 3;
    //for (int i = 0; i < 256; ++i) {
    //    *(uint32_t*)(PAGE_DIRECTORY_PADDR + 12 * KB + i * 4) = i * 4 * KB | 3;
    //}

    map_pages(0, 0, 1 * MB);
    map_pages(KERNEL_BEG_VADDR - 1 * MB, 0, 64 * MB);
    //PageDirectory* page_directory = (PageDirectory*)PAGE_DIRECTORY_PADDR;

    asm volatile(
            "mov eax, %0;"
            "mov cr3, eax;"
            "mov eax, cr0;"
            "or eax, 0x80000000;"
            "mov cr0, eax;"
            :: "i"(PAGE_DIRECTORY_PADDR)
            );
    asm("mov eax, %0; jmp eax" :: "i"(KERNEL_BEG_VADDR));
}
