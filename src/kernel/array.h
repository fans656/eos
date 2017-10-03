#ifndef ARRAY_H
#define ARRAY_H

#include "def.h"

typedef struct _Array _Array;
typedef struct _Array* Array;

Array array_new(size_t capacity);
void array_free(Array a);

void array_append(Array a, void* data);
void array_prepend(Array a, void* data);

void* array_pop(Array a);
void* array_popleft(Array a);

bool array_empty(Array a);
bool array_full(Array a);
size_t array_size(Array a);
void* array_get(Array a, size_t i);

void array_dump(Array a);

#endif
