#include "filesystem.h"
#include "string.h"
#include "disk.h"
#include "memory.h"
#include "stdio.h"
#include "math.h"
#include "time.h"

#define BLOCK_SIZE 4096
#define RESERVED_BLOCK 256
#define BITMAP_BLOCK RESERVED_BLOCK
#define BITMAP_BLOCKS 8  // 8 blocks bitmap support 1GB file
#define ROOT_BLOCK (BITMAP_BLOCK + BITMAP_BLOCKS)
#define FILEENTRY_BLOCK (ROOT_BLOCK + 1)

#define BITMAP_BYTES (BITMAP_BLOCKS * BLOCK_SIZE)

#define MAX_NAME_LEN 256
#define MAX_BLOCKSRANGE_PER_FILE 256

typedef struct {
    uint beg;
    uint count;
} BlocksRange;

typedef struct {
    uint block;
    uint next;
    char name[MAX_NAME_LEN];
    size_t size;
    size_t n_blocks;
    BlocksRange blocks[MAX_BLOCKSRANGE_PER_FILE];
} FileEntry;

typedef struct FILE {
    FileEntry* entry;
    size_t pos;
} FILE;

uchar bitmap[BITMAP_BLOCKS * BPB];

void bitmap_set(uint block, bool used) {
    if (used) {
        bitmap[block / 8] |= 1 << (block % 8);
    } else {
        bitmap[block / 8] &= ~(1 << (block % 8));
    }
}

void bitmap_range_set(uint beg, uint end, bool used) {
    while (beg % 8) {
        bitmap_set(beg++, used);
    }
    while (end % 8) {
        bitmap_set(--end, used);
    }
    for (int i = beg / 8; i < end / 8; ++i) {
        bitmap[i] = used ? 0xff : 0;
    }
}

void bitmap_new() {
    read_bytes(BITMAP_BLOCK * BPB, BITMAP_BLOCKS * BPB, bitmap);
    bitmap_range_set(0, FILEENTRY_BLOCK, true);
}

uint alloc_block() {
    for (int i = 0; i < BITMAP_BYTES; ++i) {
        if (bitmap[i] != 0xff) {
            for (int j = 0; j < 8; ++j) {
                if (bitmap[i] & (1 << j)) {
                    bitmap[i] |= (1 << j);
                    write_bytes(BITMAP_BLOCK * BPB + i, 1, &bitmap[i]);
                    return i * 8 + j;
                }
            }
        }
    }
    panic("alloc_block(): no more block!");
}

// try allocate `cnt` continuous disk data blocks and save the info in `rg`
// may allocate less than `cnt` blocks
void alloc_blocks(uint cnt, BlocksRange* rg) {
    int beg = FILEENTRY_BLOCK + 1;
    int end;
    while (bitmap[beg / 8] == 0xff) {
        beg += 8;
    }
    while (bitmap[beg / 8] & (1 << (beg % 8))) {
        ++beg;
    }
    end = beg;
    while (end / 8 < BITMAP_BYTES && cnt--
            && !(bitmap[end / 8] & (1 << (end % 8)))) {
        ++end;
    }
    rg->beg = beg;
    rg->count = end - beg;
}

FileEntry* fe_from_block(uint block) {
    FileEntry* fe = (FileEntry*)named_malloc(sizeof(FileEntry), "FileEntry");
    read_bytes(block * BPB, sizeof(FileEntry), fe);
    return fe;
}

FileEntry* fe_next_block(FileEntry* fe) {
    read_bytes(fe->next * BPB, sizeof(FileEntry), fe);
    return fe;
}

FileEntry* fe_create(const char* name) {
    uint block = alloc_block();
    FileEntry* fe = fe_from_block(block);
    memset(fe, 0, sizeof(FileEntry));
    fe->block = block;
    strcpy(fe->name, name);
    fe->size = 0;
    write_bytes(fe->block * BPB, sizeof(FileEntry), fe);
    return fe;
}

FileEntry* fe_from_name(const char* name) {
    FileEntry* p = fe_from_block(FILEENTRY_BLOCK);
    while (p) {
        if (strcmp(p->name, name) == 0) {
            return p;
        }
        if (!p->next) {
            free(p);
            return 0;
        }
        p = fe_next_block(p);
    }
}

void fe_reserve(FileEntry* fe, size_t size) {
    if (size <= fe->n_blocks * BPB) {
        return;
    }
    BlocksRange* rg = &fe->blocks[fe->n_blocks];
    int n = (size + BPB - 1) / BPB;
    if (n + fe->n_blocks > MAX_BLOCKSRANGE_PER_FILE) {
        panic("fe_reserve: MAX_BLOCKSRANGE_PER_FILE reached!");
    }
    while (n) {
        alloc_blocks(n, rg);
        n -= rg->count;
        ++rg;
    }
}

FILE* fopen(const char* name, const char* mode) {
    FileEntry* fe = fe_from_name(name);
    if (mode[0] == 'r' && !fe) {
        panic("fopen: %s does not exist\n", name);
    }
    if (!fe) {
        fe = fe_create(name);
    }
    FILE* fp = (FILE*)named_malloc(sizeof(FILE), "FILE");
    fp->entry = fe;
    fp->pos = 0;
    return fp;
}

int fclose(FILE* fp) {
    free(fp->entry);
    free(fp);
    return 0;
}

void fseek(FILE* fp, size_t pos) {
    fp->pos = pos;
}

size_t ftell(FILE* fp) {
    return fp->pos;
}

size_t fread(void* buffer, size_t size, size_t count, FILE* fp) {
    size *= count;
    FileEntry* fe = fp->entry;
    BlocksRange* rg = fe->blocks;
    uchar* p = (uchar*)buffer;
    size_t beg = 0;
    for (int i = 0; i < fe->n_blocks; ++i) {
        size_t end = beg + rg->count * BPB;
        if (beg <= fp->pos && fp->pos < end) {
            size_t offset = fp->pos - beg;
            size_t n = min(fp->pos + size, end) - fp->pos;
            read_bytes(rg->beg * BPB + offset, n, p);
            fp->pos += size;
            p += n;
            size -= n;
        }
        beg = end;
        ++rg;
    }
    if (size) {
        panic("fread: partial read");
    }
}

size_t fwrite(const void* data, size_t size, size_t count, FILE* fp) {
    size *= count;
    FileEntry* fe = fp->entry;
    fe_reserve(fe, fp->pos + size);
    BlocksRange* rg = fe->blocks;
    const uchar* p = (const uchar*)data;
    size_t beg = 0;
    for (int i = 0; i < fe->n_blocks; ++i) {
        size_t end = beg + rg->count * BPB;
        if (beg <= fp->pos && fp->pos < end) {
            size_t offset = fp->pos - beg;
            size_t n = min(fp->pos + size, end) - fp->pos;
            write_bytes(rg->beg * BPB + offset, n, p);
            fp->pos += size;
            p += n;
            size -= n;
        }
        beg = end;
        ++rg;
    }
    if (size) {
        panic("fread: partial read");
    }
}

void* load_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    size_t size = fsize(fp);
    void* data = named_malloc(size, path);
    fread(data, size, 1, fp);
    fclose(fp);
    return data;
}

size_t fsize(FILE* fp) {
    return fp->entry->size;
}

size_t fsize_from_name(char* fname) {
    return fe_from_name(fname)->size;
}

void init_filesystem() {
    init_disk();
    bitmap_new();
}

void dump_fileentry(FILE* fp) {
    FileEntry* fe = fp->entry;
    printf("======================================== FileEntry\n");
    printf("name: %s, size: %d\n", fe->name, fe->size);
    BlocksRange* rg = fe->blocks;
    for (int i = 0; i < fe->n_blocks; ++i) {
        printf("Beg: %x, Count: %x\n", rg->beg, rg->count);
        ++rg;
    }
    printf("========================================\n");
}
