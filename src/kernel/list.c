#include "list.h"
#include "memory.h"

typedef struct Node Node;

typedef struct Node {
    Node* prev;
    Node* next;
    void* data;
} Node;

typedef struct _List {
    Node* head;
    Node* tail;
    size_t size;
} _List;

Node* node_new(void* data) {
    Node* p = named_malloc(sizeof(Node), "Node");
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
    Node* p = l->head;
    while (p) {
        Node* next = p->next;
        free(p);
        p = next;
    }
    free(l);
}

static void insert_after(Node* p, Node* q) {
    q->prev = p;
    q->next = p->next;
    q->prev->next = q;
    q->next->prev = q;
}

static Node* take(Node* p) {
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
    Node* p = take(l->tail->prev);
    void* data = p->data;
    free(p);
    --l->size;
    return data;
}

void* list_popleft(List l) {
    Node* p = take(l->head->next);
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
