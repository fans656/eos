#ifndef FILESYSTEM_H
#define FILESYSTEM_H

typedef struct {
    uint32_t type;
    uint32_t i_parent;
    uint32_t i_prev_sibling;
    uint32_t i_next_sibling;
    uint32_t name_len;
    char name[492];
} FilesystemEntry;

typedef struct {
    FilesystemEntry entry;
    uint64_t size;
    uint32_t i_next_entry;
    uint32_t n_data_blocks;
    uint32_t* data_blocks;
} FilesystemFileEntry;

typedef struct FILE {
    FilesystemFileEntry* entry;
    FilesystemFileEntry* cur_pos_entry;
    int ith_entry;
    uint64_t pos;
} FILE;

void init_filesystem();

void ls();
void tree();

struct FILE* fopen(char* fpath);
void fclose(struct FILE* fp);

void fseek(struct FILE* fp, uint64_t offset, int anchor);
uint64_t ftell(struct FILE* fp);

void fread(struct FILE* fp, uint64_t n_bytes, char* buffer);

#endif
