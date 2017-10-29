#ifndef DICT_H
#define DICT_H

#include "list.h"

template<typename T>
size_t hash(T* key, size_t size) {
    return (uint)key % size;
}

template<typename T>
size_t hash(T key, size_t size) {
    return (uint)key % size;
}

constexpr size_t INITIAL_DICT_SIZE = 11;

template<typename Key, typename Val>
struct Dict {
    struct Pair {
        Key key;
        Val val;
    };

    Dict() {
        construct();
    }
    
    void construct() {
        capacity = INITIAL_DICT_SIZE;
        a = new List<Pair>[capacity];
        size_ = 0;
    }
    
    bool has_key(const Key& key) const {
        auto l = get_list(key);
        for (auto kv: *l) {
            if (kv.key == key) {
                return true;
            }
        }
        return false;
    }
    
    Val operator[](const Key& key) const {
        auto l = get_list(key);
        for (auto kv: *l) {
            if (kv.key == key) {
                return kv.val;
            }
        }
        return 0;
    }
    
    void add(const Key& key, const Val& val) {
        auto l = get_list(key);
        l->append(Pair{key, val});
    }
    
    List<Pair>* get_list(const Key& key) const {
        return &a[hash(key, capacity)];
    }
    
    size_t size() const { return size_; }
    
    size_t capacity;
    size_t size_;
    List<Pair>* a;
};

#endif
