#include "list.h"
#include "../kernel/memory.h"

typedef struct _List {
    ListNode* head;
    ListNode* tail;
    size_t size;
} _List;

ListNode* node_new(void* data) {
    ListNode* p = named_malloc(sizeof(ListNode), "Node");
    p->prev = p->next = 0;
    p->data = data;
    return p;
}

List list_new() {
    List l = named_malloc(sizeof(_List), "List");
    l->size = 0;
    l->head = node_new(0);
    l->tail = node_new(0);
    l->head->next = l->tail;
    l->tail->prev = l->head;
    return l;
}

void list_free(List l) {
    ListNode* p = l->head;
    while (p) {
        ListNode* next = p->next;
        free(p);
        p = next;
    }
    free(l);
}

static void insert_after(ListNode* p, ListNode* q) {
    q->prev = p;
    q->next = p->next;
    q->prev->next = q;
    q->next->prev = q;
}

static ListNode* take(ListNode* p) {
    p->prev->next = p->next;
    p->next->prev = p->prev;
    return p;
}

void list_append(List l, void* data) {
    insert_after(l->tail->prev, node_new(data));
    ++l->size;
}

void list_prepend(List l, void* data) {
    insert_after(l->head, node_new(data));
    ++l->size;
}

void* list_pop(List l) {
    ListNode* p = take(l->tail->prev);
    void* data = p->data;
    free(p);
    --l->size;
    return data;
}

void* list_popleft(List l) {
    ListNode* p = take(l->head->next);
    void* data = p->data;
    free(p);
    --l->size;
    return data;
}

bool list_empty(List l) {
    return l->size == 0;
}

size_t list_size(List l) {
    return l->size;
}

ListIter list_iter(List l) {
    return l->head->next;
}

ListIter list_riter(List l) {
    return l->tail->prev;
}

bool list_iter_valid(ListIter iter) {
    return iter->prev != 0 && iter->next != 0;
}

ListIter list_iter_prev(ListIter iter) {
    return iter->prev;
}

ListIter list_iter_next(ListIter iter) {
    return iter->next;
}
