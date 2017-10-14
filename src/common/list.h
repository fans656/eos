#ifndef LIST_H
#define LIST_H

#include "def.h"

template<typename T>
struct List {
    struct Node {
        Node* prev;
        Node* next;
        T data;
        
        Node(T data = T()) : prev(0), next(0), data(data) {}
        void prepend(Node* node) {
            node->prev = prev;
            node->next = this;
            prev->next = prev = node;
        }
        void append(Node* node) {
            node->prev = this;
            node->next = next;
            next->prev = next = node;
        }
        Node* take() {
            prev->next = next;
            next->prev = prev;
            return this;
        }
    };

    struct iter {
        Node* node;
        iter(Node* node) : node(node) {}
        T operator*() { return node->data; }
        iter& operator++() { node = node->next; return *this; }
        iter& operator--() { node = node->prev; return *this; }
        bool operator==(iter o) { return node == o.node; }
        bool operator!=(iter o) { return !(*this == o); }
        void remove() { node->take(); }
    };

    struct riter {
        Node* node;
        riter(Node* node) : node(node) {}
        T operator*() { return node->data; }
        riter& operator++() { node = node->prev; return *this; }
        riter& operator--() { node = node->next; return *this; }
        bool operator==(riter o) { return node == o.node; }
        bool operator!=(riter o) { return !(*this == o); }
        void remove() { node->take(); }
    };

    Node* head;
    Node* tail;
    size_t size_;
    
    List() { construct(); }
    ~List() { desctruct(); }
    
    void construct() {
        head = new Node();
        tail = new Node();
        head->next = tail;
        tail->prev = head;
        size_ = 0;
    }
    
    void desctruct() {
        Node* p = head;
        while (p) {
            Node* next = p->next;
            delete p;
            p = next;
        }
    }
    
    void prepend(T data) {
        ++size_;
        head->append(new Node(data));
    }
    
    void append(T data) {
        ++size_;
        tail->prepend(new Node(data));
    }
    
    T pop() {
        --size_;
        return tail->prev->take()->data;
    }
    
    T popleft() {
        --size_;
        return head->next->take()->data;
    }
    
    bool remove(const T& target) {
        auto it = begin();
        for (auto t: *this) {
            if (t == target) {
                it.remove();
                return true;
            }
            ++it;
        }
        return false;
    }
    
    iter find(const T& target) {
        auto it = begin();
        for (auto x: *this) {
            if (x == target) {
                return it;
            }
            ++it;
        }
        return it;
    }
    
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    iter begin() { return iter(head->next); }
    iter end() { return iter(tail); }
    riter rbegin() { return riter(tail->prev); }
    riter rend() { return riter(head); }
};

#endif
