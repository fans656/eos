#include "def.h"

void* memset(void* ptr, uchar value, uint cnt);
void* memmove(void* dst, const void* src, uint cnt);
#define memcpy memmove

void* strcpy(char* dst, const char* src);
void* strncpy(char* dst, const char* src, size_t n);
size_t strlen(const char* s);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
