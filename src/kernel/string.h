#include "../types.h"

void* memset(void* ptr, uchar value, uint cnt);
void* memcpy(void* dst, void* src, uint cnt);

void* strcpy(char* dst, char* src);
void* strncpy(char* dst, char* src, size_t n);
size_t strlen(char* s);
int strcmp(char* s1, char* s2);
int strncmp(char* s1, char* s2, size_t n);
