#ifndef ARRAY_H
#define ARRAY_H

#include "def.h"

constexpr size_t INITIAL_CAPACITIY = 8;

template<typename T>
struct Array {
    Array() { construct(); }
    ~Array() { destruct(); }
    
    void construct() {
        capacity = INITIAL_CAPACITIY;
        size_ = 0;
        a = new T[INITIAL_CAPACITIY];
        beg_ = 0;
        end_ = 0;
    }
    
    void destruct() {
        delete[] a;
    }
    
    inline size_t size() const { return size_; }
    inline bool empty() const { return size_ == 0; }
    
    void append(T t) {
        if (size_ + 1 == capacity) reserve(capacity * 2);
        a[end_] = t;
        end_ = inc(end_);
        ++size_;
    }
    
    void prepend(T t) {
        if (size_ + 1 == capacity) reserve(capacity * 2);
        beg_ = dec(beg_);
        a[beg_] = t;
        ++size_;
    }
    
    T& pop() {
        --size_;
        end_ = dec(end_);
        return a[end_];
    }
    
    T& popleft() {
        --size_;
        size_t i = beg_;
        beg_ = inc(beg_);
        return a[i];
    }
    
    T& operator[](size_t i) {
        return a[(beg_ + i) % capacity];
    }
    
    void reserve(size_t new_capacity) {
        T* aa = new T[new_capacity];
        size_t i = 0;
        for (auto t: *this) {
            aa[i++] = t;
        }
        beg_ = 0;
        end_ = size_;
        capacity = new_capacity;
        delete[] a;
        a = aa;
    }
    
    struct iter {
        T operator*() { return arr.a[i]; }
        iter& operator++() { i = arr.inc(i); return *this; }
        iter& operator--() { i = arr.dec(i); return *this; }
        bool operator==(const iter& o) { return i == o.i; }
        bool operator!=(const iter& o) { return i != o.i; }
        iter(Array& arr, size_t i) : arr(arr), i(i) {}
        Array& arr;
        size_t i;
    };
    
    struct riter {
        T operator*() { return arr.a[arr.dec(i)]; }
        riter& operator++() { i = arr.dec(i); return *this; }
        riter& operator--() { i = arr.inc(i); return *this; }
        bool operator==(const riter& o) { return i == o.i; }
        bool operator!=(const riter& o) { return i != o.i; }
        riter(Array& arr, size_t i) : arr(arr), i(i) {}
        Array& arr;
        size_t i;
    };
    
    iter begin() { return iter(*this, beg_); }
    iter end() { return iter(*this, end_); }
    riter rbegin() { return riter(*this, end_); }
    riter rend() { return riter(*this, beg_); }
    
    inline size_t inc(size_t i) { return (i + 1) % capacity; }
    inline size_t dec(size_t i) { return (i + capacity - 1) % capacity; }
    
    size_t capacity;
    size_t size_;
    size_t beg_;
    size_t end_;
    T* a;
};

#endif
