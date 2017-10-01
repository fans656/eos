#include "filesystem.h"
#include "string.h"
#include "disk.h"
#include "memory.h"
#include "stdio.h"
#include "math.h"

#define BLOCK_SIZE 4096
#define RESERVED_BLOCK 256
#define BITMAP_BLOCK RESERVED_BLOCK
#define BITMAP_BLOCKS 8  // 8 blocks bitmap support 1GB file
#define ROOT_BLOCK (BITMAP_BLOCK + BITMAP_BLOCKS)
#define FILEENTRY_BLOCK (ROOT_BLOCK + 1)

#define BITMAP_BYTES (BITMAP_BLOCKS * BLOCK_SIZE)

#define MAX_NAME_LEN 256

typedef struct {
    uint beg;
    uint count;
} BlocksRange;

typedef struct {
    uint block;
    uint next;
    char name[MAX_NAME_LEN];
    size_t size;
    BlocksRange blocks[512];
} FileEntry;

typedef struct FILE {
    FileEntry* entry;
    size_t pos;
} FILE;

uchar* bitmap;

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
    bitmap = malloc(BITMAP_BLOCKS * BPB);
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
    FileEntry* fe = malloc(BLOCK_SIZE);
    read_bytes(block * BPB, BLOCK_SIZE, fe);
    return fe;
}

FileEntry* fe_next_block(FileEntry* fe) {
    return fe_from_block(fe->next);
}

FileEntry* fe_create(char* name) {
    FileEntry* fe = fe_from_block(alloc_block());
    memset(fe, 0, sizeof(FileEntry));
    strcpy(fe->name, name);
    fe->size = 0;
    write_bytes(fe->block * BPB, BLOCK_SIZE, fe);
    return fe;
}

FileEntry* fe_from_name(char* name) {
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
    if (size < fe->size) {
        return;
    }
    size_t sz = 0;
    BlocksRange* rg = &fe->blocks[0];
    while (sz < size) {
        if (rg->count) {
            sz += rg->count * BPB;
        } else {
            alloc_blocks((size - sz + BLOCK_SIZE - 1) / BLOCK_SIZE, rg);
            sz += rg->count * BPB;
        }
        ++rg;
    }
}

FILE* fopen(char* name) {
    FileEntry* fe = fe_from_name(name);
    if (!fe) {
        fe = fe_create(name);
    }
    FILE* fp = malloc(sizeof(FILE));
    fp->entry = fe;
    return fp;
}

void fclose(FILE* fp) {
    free(fp->entry);
    free(fp);
}

void fseek(FILE* fp, size_t pos) {
    fp->pos = pos;
}

size_t ftell(FILE* fp) {
    return fp->pos;
}

void fread(FILE* fp, size_t size, void* data) {
    BlocksRange* rg = &fp->entry->blocks[0];
    uchar* p = data;
    size_t beg = 0;
    while (size) {
        size_t end = beg + rg->count * BPB;
        if (beg <= fp->pos && fp->pos < end) {
            size_t offset = fp->pos - beg;
            size_t n = min(fp->pos + size, end) - fp->pos;
            read_bytes(rg->beg * BPB + offset, n, p);
            fp->pos += size;
            p += n;
            size -= n;
        }
    }
}

void fwrite(FILE* fp, void* data, size_t size) {
    fe_reserve(fp->entry, fp->pos + size);
    BlocksRange* rg = &fp->entry->blocks[0];
    uchar* p = data;
    size_t beg = 0;
    while (size) {
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
}

size_t fsize(char* name) {
    return fe_from_name(name)->size;
}

void init_filesystem() {
    init_disk();
    bitmap_new();
}
