#ifndef LIST_H
#define LIST_H

#include "def.h"
#include "../kernel/stdio.h"

template<typename T>
struct List {
    struct DataNode;

    struct Node {
        Node() : prev(0), next(0) {}
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
        DataNode* take() {
            prev->next = next;
            next->prev = prev;
            return (DataNode*)this;
        }
        
        Node* prev;
        Node* next;
    };
    
    struct DataNode : public Node {
        T data;
        DataNode(T data = T()) : Node(), data(data) {}
    };

    struct iter {
        Node* node;
        iter(Node* node) : node(node) {}
        const T& operator*() const { return ((DataNode*)node)->data; }
        T& operator*() { return ((DataNode*)node)->data; }
        iter& operator++() { node = node->next; return *this; }
        iter& operator--() { node = node->prev; return *this; }
        bool operator==(iter o) { return node == o.node; }
        bool operator!=(iter o) { return !(*this == o); }
        void remove() { delete node->take(); }
    };

    struct riter {
        Node* node;
        riter(Node* node) : node(node) {}
        const T& operator*() const { return ((DataNode*)node)->data; }
        T& operator*() { return ((DataNode*)node)->data; }
        riter& operator++() { node = node->prev; return *this; }
        riter& operator--() { node = node->next; return *this; }
        bool operator==(riter o) { return node == o.node; }
        bool operator!=(riter o) { return !(*this == o); }
        void remove() { delete node->take(); }
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
        head->append(new DataNode(data));
    }
    
    void append(T data) {
        ++size_;
        tail->prepend(new DataNode(data));
    }
    
    void extend(List<T> l) {
        for (auto& x: l) {
            append(x);
        }
    }
    
    void clear() {
        while (!empty()) {
            pop();
        }
    }
    
    T pop() {
        --size_;
        auto node = tail->prev->take();
        auto res = &node->data;
        delete node;
        return *res;
    }
    
    T peekleft() {
        return ((DataNode*)head->next)->data;
    }
    
    T& popleft() {
        --size_;
        auto node = head->next->take();
        auto res = &node->data;
        delete node;
        return *res;
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
    
    riter rfind(const T& target) {
        auto it = rbegin();
        for (; it != rend(); ++it) {
            if (*it == target) {
                return it;
            }
        }
        return it;
    }
    
    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    iter begin() { return iter(head->next); }
    iter end() { return iter(tail); }
    riter rbegin() { return riter(tail->prev); }
    riter rend() { return riter(head); }
    
    const T& first() const { return *begin(); }
    T& first() { return *begin(); }
    
    const T& last() const { return *rbegin(); }
    T& last() { return *rbegin(); }
    
    List<T> clone() {
        List<T> l;
        for (const auto& x: *this) {
            l.append(x);
        }
        return l;
    }
};

#endif
