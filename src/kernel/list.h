#ifndef LIST_H
#define LIST_H

#include "def.h"

typedef struct _List _List;
typedef struct _List* List;

List list_new();
void list_free(List l);

void list_append(List l, void* data);
void list_prepend(List l, void* data);

void* list_pop(List l);
void* list_popleft(List l);

bool list_empty(List l);
size_t list_size(List l);

#endif
