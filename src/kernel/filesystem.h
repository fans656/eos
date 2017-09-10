#ifndef FILESYSTEM_H
#define FILESYSTEM_H

typedef struct {
    uint32_t i_entry;
    uint64_t pos;
} FILE;

void init_filesystem();

void ls();
void tree();

FILE* fopen(char* path);
void fclose(FILE* fp);

void fseek(FILE* fp, uint64_t offset, int anchor);
uint64_t ftell(FILE* fp);

void fread(FILE* fp, char* buffer, uint64_t n_bytes);
void fwrite(FILE* fp, char* data, uint64_t n_bytes);

#endif
