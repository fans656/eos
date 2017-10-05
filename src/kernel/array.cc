#include "array.h"
#include "../kernel/memory.h"
#include "../kernel/stdio.h"

typedef struct _Array {
    void** a;
    size_t capacity;
    size_t size;
    size_t beg;
    size_t end;
}_Array;

Array array_new(size_t capacity) {
    Array a = (Array)named_malloc(sizeof(_Array), "Array");
    a->a = (void**)named_malloc(capacity * sizeof(void*), "Array.a");
    a->capacity = capacity;
    a->size = 0;
    a->beg = a->end = 0;
    return a;
}

void array_free(Array a) {
    free(a->a);
    free(a);
}

void array_append(Array a, void* data) {
    size_t end = a->end;
    a->a[end] = data;
    end = (end + 1) % a->capacity;
    a->end = end;
    ++a->size;
}

void array_prepend(Array a, void* data) {
    size_t beg = a->beg;
    size_t capacity = a->capacity;
    beg = (beg + capacity - 1) % capacity;
    a->a[beg] = data;
    a->beg = beg;
    ++a->size;
}

void* array_pop(Array a) {
    size_t end = a->end;
    size_t capacity = a->capacity;
    end = (end + capacity - 1) % capacity;
    a->end = end;
    --a->size;
    return a->a[end];
}

void* array_popleft(Array a) {
    size_t beg = a->beg;
    size_t capacity = a->capacity;
    void* res = a->a[beg];
    a->beg = (beg + 1) % capacity;
    --a->size;
    return res;
}

bool array_empty(Array a) {
    return a->size == 0;
}

bool array_full(Array a) {
    return a->size == a->capacity;
}

size_t array_size(Array a) {
    return a->size;
}

void* array_get(Array a, size_t i) {
    return a->a[(a->beg + i) % a->capacity];
}

void* array_remove(Array arr, size_t i) {
    int end = arr->end;
    int capacity = arr->capacity;
    void** a = arr->a;

    int last = (end + capacity - 1) % capacity;
    void* res = a[i];
    a[i] = a[last];
    arr->end = last;
    --arr->size;

    return res;
}

void array_dump(Array a) {
    printf("%d/%d [%d, %d)\n", a->size, a->capacity, a->beg, a->end);
    for (int i = 0; i < 5; ++i) {
        printf("%x ", a->a[i]);
    }
    putchar('\n');
}
