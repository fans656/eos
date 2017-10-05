#ifndef LIST_H
#define LIST_H

#include "def.h"

typedef struct ListNode ListNode;

typedef struct ListNode {
    ListNode* prev;
    ListNode* next;
    void* data;
} ListNode;

typedef struct _List _List;
typedef struct _List* List;
typedef struct ListNode* ListIter;

List list_new();
void list_free(List l);

void list_append(List l, void* data);
void list_prepend(List l, void* data);

void* list_pop(List l);
void* list_popleft(List l);

bool list_empty(List l);
size_t list_size(List l);

ListIter list_iter(List l);
ListIter list_riter(List l);
bool list_iter_valid(ListIter iter);
ListIter list_iter_prev(ListIter iter);
ListIter list_iter_next(ListIter iter);

#endif
