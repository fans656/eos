#include <stdint.h>
#include "filesystem.h"
#include "disk.h"
#include "io.h"
#include "math.h"
#include "malloc.h"

#define I_META 256
#define I_BITMAP (I_META + 1)

#define TYPE_DIRECTORY 0x01
#define TYPE_FILE 0x02

#define MAX_DATA_BLOCKS_PER_ENTRY ((4096 - 1024) / 4)
#define BYTES_PER_ENTRY (MAX_DATA_BLOCKS_PER_ENTRY * BPP)

typedef FilesystemEntry Entry;
typedef FilesystemFileEntry FileEntry;

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

uint8_t* bitmap;

void read_meta() {
    read_bytes(I_META * BPP, sizeof(Meta), &meta);
}

void read_bitmap() {
    uint32_t n_bytes_bitmap = (meta.n_blocks + 7) / 8;
    bitmap = malloc(n_bytes_bitmap);
    read_bytes(I_BITMAP * BPP, n_bytes_bitmap, bitmap);
}

void init_filesystem() {
    init_disk();
    if (disk_meta.bytes_per_sector != BYTES_PER_SECTOR) {
        panic("Unsupported bytes per sector: %d\n", disk_meta.bytes_per_sector);
    }
    read_meta();
    read_bitmap();

    int n_bytes_bitmap = meta.n_blocks / 8;
    int n_blocks_bitmap = (n_bytes_bitmap + BPP - 1) / BPP;
    filesystem.i_root_directory = I_BITMAP + n_blocks_bitmap;
    filesystem.i_current_directory = filesystem.i_root_directory;
}

////////////////////////////////////////////////////////////

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

typedef struct {
    Directory entry;
    uint32_t i_block;
} Iter;

Iter* iter_new(int i_block) {
    Iter* this = malloc(sizeof(Iter));
    read_bytes(i_block * BPP, sizeof(Directory), this);
    this->i_block = i_block;
    return this;
}

Iter* first_child(Iter* this) {
    uint32_t i_first_child = ((Directory*)this)->i_first_child;
    if (!i_first_child) {
        return 0;
    }
    read_bytes(i_first_child * BPP, sizeof(Directory), this);
    this->i_block = i_first_child;
    return this;
}

Iter* next_sibling(Iter* this) {
    uint32_t i_next_sibling = ((Entry*)this)->i_next_sibling;
    if (!i_next_sibling) {
        return 0;
    }
    read_bytes(i_next_sibling * BPP, sizeof(Directory), this);
    this->i_block = i_next_sibling;
    return this;
}

Iter* iter_clone(Iter* this) {
    Iter* res = malloc(sizeof(Iter));
    return iter_new(this->i_block);
}

////////////////////////////////////////////////////////////

void pwd() {
}

void print_entry_name(Entry* entry) {
    printf("%s%c\n", entry->name, entry_is_dir(entry) ? '/' : 0);
}

void ls() {
    Iter* p = iter_new(filesystem.i_current_directory);
    p = first_child(p);
    while (p) {
        print_entry_name((Entry*)p);
        p = next_sibling(p);
    }
}

void _tree(Iter* cur, int depth) {
    for (int i = 0; i < depth; ++i) {
        printf("    ");
    }
    print_entry_name((Entry*)cur);
    if (entry_is_dir((Entry*)cur)) {
        cur = first_child(cur);
        while (cur) {
            _tree(iter_clone(cur), depth + 1);
            cur = next_sibling(cur);
        }
    }
}

void tree() {
    _tree(iter_new(filesystem.i_current_directory), 0);
}

////////////////////////////////////////////////////////////

FileEntry* file_entry_new(uint32_t i_block) {
    FileEntry* this = malloc(sizeof(FileEntry));
    read_bytes(i_block * BPP, sizeof(FileEntry) - 4, this);
    return this;
}

uint32_t get_file_entry_by_fpath(char* fpath) {
    Iter* cur = iter_new(filesystem.i_root_directory);
    char* name_beg = fpath + 1;
    char* name_end = name_beg;
    while (true) {
        while (*name_end && *name_end != '/') {
            ++name_end;
        }
        cur = first_child(cur);
        while (cur) {
            Entry* entry = (Entry*)cur;
            if (strncmp(entry->name, name_beg, entry->name_len) == 0) {
                break;
            }
            cur = next_sibling(cur);
        }
        if (!cur) {
            return 0;
        }
        if (!*name_end) {
            break;
        }
        name_beg = name_end + 1;
        name_end = name_beg;
    }
    return cur->i_block;
}

FileEntry* next_file_entry(FileEntry* this) {
    uint32_t i_next_entry = this->i_next_entry;
    if (!i_next_entry) {
        return 0;
    }
    read_bytes(i_next_entry * BPP, sizeof(FileEntry) - 4, this);
    return this;
}

void load_data_blocks_table(uint32_t i_entry, FileEntry* entry) {
    read_bytes(i_entry * BPP + 1024, entry->n_data_blocks * 4, entry->data_blocks);
}

FILE* fopen(char* fpath) {
    uint32_t i_entry = get_file_entry_by_fpath(fpath);
    if (!i_entry) {
        return 0;
    }

    FILE* fp = malloc(sizeof(FILE));
    fp->entry = file_entry_new(i_entry);
    fp->cur_pos_entry = file_entry_new(i_entry);
    fp->cur_pos_entry->data_blocks = malloc(MAX_DATA_BLOCKS_PER_ENTRY * 4);
    load_data_blocks_table(i_entry, fp->cur_pos_entry);
    fp->ith_entry = 0;
    fp->pos = 0;

    return fp;
}

void fclose(FILE* fp) {
}

void fseek(FILE* fp, uint64_t offset, int anchor) {
    fp->pos = offset;
}

uint64_t ftell(FILE* fp) {
}

uint64_t fsize(FILE* fp) {
    return fp->entry->size;
}

void fread(FILE* fp, uint64_t n_bytes, void* buffer) {
    uint64_t pos = fp->pos;
    n_bytes = min(fp->entry->size - fp->pos, n_bytes);
    while (n_bytes) {
        uint32_t ith_entry = pos / BYTES_PER_ENTRY;
        uint32_t i_byte_in_entry = pos % BYTES_PER_ENTRY;
        if (fp->ith_entry != ith_entry) {
            if (fp->ith_entry > ith_entry) {
                panic("back seek not supported\n");
            }
            for (int i = fp->ith_entry; i < ith_entry; ++i) {
                fp->cur_pos_entry = next_file_entry(fp->cur_pos_entry);
            }
            load_data_blocks_table(ith_entry, fp->cur_pos_entry);
            fp->ith_entry = ith_entry;
        }
        uint32_t ith_block = i_byte_in_entry / BPP;
        uint32_t i_block = fp->cur_pos_entry->data_blocks[ith_block];
        uint64_t n_read = min(n_bytes, BYTES_PER_BLOCK);
        read_bytes(i_block * BPP, n_read, buffer);
        n_bytes -= n_read;
        pos += n_read;
        buffer += n_read;
    }
    fp->pos = pos;
}
