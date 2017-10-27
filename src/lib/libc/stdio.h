#ifndef STDIO_H
#define STDIO_H

#include "../common/def.h"

typedef struct FILE FILE;

int printf(const char* fmt, ...);
int debug(const char* fmt, ...);
void panic(const char* fmt, ...);

FILE* fopen(const char* path, const char* mode);
int fclose(FILE* fp);
size_t fread(void* buffer, size_t size, size_t count, FILE* fp);
size_t fwrite(const void* data, size_t size, size_t count, FILE* fp);
size_t fsize(FILE* fp);
void* load_file(const char* path);

void hexdump(const void* addr, uint cnt);

#endif
