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

    struct Iter {
        Node* node;
        Iter(Node* node) : node(node) {}
        T operator*() { return node->data; }
        Iter& operator++() { node = node->next; return *this; }
        Iter& operator--() { node = node->prev; return *this; }
        bool operator==(Iter o) { return node == o.node; }
        bool operator!=(Iter o) { return !(*this == o); }
    };

    struct RIter {
        Node* node;
        RIter(Node* node) : node(node) {}
        T operator*() { return node->data; }
        RIter& operator++() { node = node->prev; return *this; }
        RIter& operator--() { node = node->next; return *this; }
        bool operator==(RIter o) { return node == o.node; }
        bool operator!=(RIter o) { return !(*this == o); }
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
    
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    Iter begin() { return Iter(head->next); }
    Iter end() { return Iter(tail); }
    RIter rbegin() { return RIter(tail->prev); }
    RIter rend() { return RIter(head); }
};

#endif
