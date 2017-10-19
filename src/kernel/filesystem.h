#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "def.h"

typedef struct FILE FILE;

void init_filesystem();

FILE* fopen(const char* name, const char* mode);
int fclose(FILE* fp);

void fseek(FILE* fp, size_t pos);
size_t ftell(FILE* fp);

size_t fwrite(const void* data, size_t size, size_t count, FILE* fp);
size_t fread(void* buffer, size_t size, size_t count, FILE* fp);

size_t fsize(FILE* fp);
size_t fsize_from_name(char* fname);

void* load_file(const char* path);

#endif
