#include <stdint.h>
#include "filesystem.h"
#include "disk.h"
#include "io.h"

#define I_META 256
#define I_BITMAP (I_META + 1)

#define TYPE_DIRECTORY 0x01
#define TYPE_FILE 0x02

typedef struct {
    uint32_t n_blocks;
    uint32_t i_root;
} Meta;
static Meta meta;

typedef struct {
    int i_root_directory;
    int i_current_directory;
} FileSystem;
FileSystem filesystem;

char path_buffer[4096];
FILE file_buffer;

uint8_t* bitmap = (uint8_t*)0x100000;

void read_meta() {
    read_bytes(I_META * BPP, sizeof(Meta), &meta);
}

void read_bitmap() {
    read_bytes(I_BITMAP * BPP, meta.n_blocks / 8, bitmap);
}

void init_filesystem() {
    init_disk();
    if (disk_meta.bytes_per_sector != BYTES_PER_SECTOR) {
        printf("Unsupported bytes per sector: %d\n", disk_meta.bytes_per_sector);
        panic();
    }
    read_meta();
    read_bitmap();

    int n_bytes_bitmap = meta.n_blocks / 8;
    int n_blocks_bitmap = (n_bytes_bitmap + BPP - 1) / BPP;
    filesystem.i_root_directory = I_BITMAP + n_blocks_bitmap;
    filesystem.i_current_directory = filesystem.i_root_directory;
}

////////////////////////////////////////////////////////////

typedef struct {
    uint32_t type;
    uint32_t i_parent;
    uint32_t i_prev_sibling;
    uint32_t i_next_sibling;
    uint32_t name_len;
    char name[492];
} Entry;

void entry_init(void* this, int i_block) {
    read_bytes(i_block * BPP, sizeof(Entry), this);
}

bool entry_is_dir(void* this) {
    Entry* _this = (Entry*)this;
    return _this->type == TYPE_DIRECTORY;
}

////////////////////////////////////////////////////////////

typedef struct {
    Entry entry;
    uint32_t i_first_child;
    uint32_t i_last_child;
} Directory;
typedef Directory Iter;

Iter* iter_init(void* this, int i_block) {
    read_bytes(i_block * BPP, sizeof(Iter), this);
    return this;
}

Iter* first_child(Iter* this) {
    if (!this->i_first_child) {
        return 0;
    }
    read_bytes(this->i_first_child * BPP, sizeof(Iter), this);
    return this;
}

Iter* next_sibling(Iter* this) {
    if (!this->entry.i_next_sibling) {
        return 0;
    }
    read_bytes(this->entry.i_next_sibling * BPP, sizeof(Iter), this);
    return this;
}

////////////////////////////////////////////////////////////

void pwd() {
}

void print_entry_name(Iter* entry) {
    printf("%s%c\n", entry->entry.name, entry_is_dir(entry) ? '/' : 0);
}

void ls() {
    Iter iter;
    Iter* p = &iter;
    iter_init(p, filesystem.i_current_directory);
    p = first_child(p);
    while (p) {
        print_entry_name(p);
        p = next_sibling(p);
    }
}

void _tree(int i_block, int depth) {
    if (!i_block) {
        return;
    }
    Iter iter;
    Iter* cur = &iter;
    cur = iter_init(cur, i_block);
    for (int i = 0; i < depth; ++i) {
        printf("    ");
    }
    print_entry_name(cur);
    if (entry_is_dir(cur)) {
        int i_child = cur->i_first_child;
        cur = first_child(cur);
        while (cur) {
            _tree(i_child, depth + 1);
            i_child = cur->entry.i_next_sibling;
            cur = next_sibling(cur);
        }
    }
}

void tree() {
    _tree(filesystem.i_current_directory, 0);
}

FILE* fopen(char* path) {
    ;
}

void fclose(FILE* fp) {
}

void fseek(FILE* fp, uint64_t offset, int anchor) {
}

uint64_t ftell(FILE* fp) {
}

void fread(FILE* fp, char* buffer, uint64_t n_bytes) {
}

void fwrite(FILE* fp, char* data, uint64_t n_bytes) {
}
