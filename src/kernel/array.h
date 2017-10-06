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
void* array_remove(Array arr, size_t i);

void array_dump(Array a);

#endif

/*
#ifndef ARRAY_H
#define ARRAY_H

#include "def.h"

constexpr size_t INITIAL_CAPACITIY = 8;

template<typename T>
struct Array {
    Array()
        : capacity(INITIAL_CAPACITIY), size_(0),
        elems(new T[INITIAL_CAPACITIY]), beg(0), end(0) {}
    
    inline size_t size() const { return size_; }
    inline bool empty() const { return size_ == 0; }
    
    void append(T t) {
        if (size_ + 1 == capacity) reserve(capacity * 2);
        a[end] = t;
        end = inc(end);
        ++size_;
    }
    
    void prepend(T t) {
        if (size_ + 1 == capacity) reserve(capacity * 2);
        beg = dec(beg);
        a[beg] = t;
        ++size_;
    }
    
    T& pop() {
        --size_;
        end = dec(end);
        return a[end];
    }
    
    T& popleft() {
        --size_;
        size_t i = beg;
        beg = inc(beg);
        return a[i];
    }
    
    T& operator[](size_t i) {
        return a[(beg + i) % capacity];
    }
    
    void reserve(size_t new_capacity) {
        T* aa = new T[new_capacity];
        size_t i = 0;
        for (auto& t: a) {
            aa[i++] = t;
        }
        beg = 0;
        end = size_;
        capacity = new_capacity;
        delete[] a;
        a = aa;
    }
    
    struct Iter {
        T& operator*() { return arr.a[i]; }
        Iter& operator++() { i = inc(i); return *this; }
        Iter& operator--() { i = dec(i); return *this; }
        bool operator==(const Iter& o) { return i == o.i; }
        bool operator!=(const Iter& o) { return i != o.i; }
        Iter(Array& arr, size_t i) : arr(arr), i(i) {}
        Array& arr;
        size_t i;
    };
    
    struct RIter {
        T& operator*() { return arr.a[dec(i)]; }
        RIter& operator++() { i = dec(i); return *this; }
        RIter& operator--() { i = inc(i); return *this; }
        bool operator==(const RIter& o) { return i == o.i; }
        bool operator!=(const RIter& o) { return i != o.i; }
        RIter(Array& arr, size_t i) : arr(arr), i(i) {}
        Array& arr;
        size_t i;
    };
    
    Iter begin() { return Iter(*this, beg); }
    Iter end() { return Iter(*this, end); }
    RIter rbegin() { return RIter(*this, end); }
    RIter rend() { return RIter(*this, beg); }
    
    inline size_t inc(size_t i) { return (i + 1) % capacity; }
    inline size_t dec(size_t i) { return (i + capacity - 1) % capacity; }
    
    size_t capacity;
    size_t size_;
    size_t beg;
    size_t end;
    T* a;
};

#endif
 */
