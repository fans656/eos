#include "malloc.h"
#include "memory.h"

struct BlockHeader {
    void init(size_t sz = 0) {
        used = false;
        size = sz;
        prev = next = free_prev = free_next = 0;
    }
    
    void append(BlockHeader* succ) {
        succ->prev = this;
        succ->next = next;
        next->prev = next = succ;
    }
    
    bool splitable(size_t sz) {
        return size - sz >= sizeof(BlockHeader) + 4;
    }
    
    void split(size_t sz) {
        BlockHeader* succ = (BlockHeader*)(data + sz);
        succ->init(size - sz - sizeof(BlockHeader));
        size = sz;
        append(succ);
    }
    
    void* use() {
        used = true;
        return data;
    }

    uint used;
    uint size;
    BlockHeader* prev;
    BlockHeader* next;
    BlockHeader* free_prev;
    BlockHeader* free_next;
    uchar data[0];
};

struct MallocList {
    void init() {
        head_.init();
        tail_.init();
        free_head = head = &head_;
        free_tail = tail = &tail_;
    }
    
    void* malloc(size_t size) {
        size = align4(size);
        auto p = free_head->next;
        while (p->size && p->size < size) {
            p = p->free_next;
        }
        if (!p->size) {
            p = new_block(size);
        }
        if (p->splitable(size)) {
            p->split(size);
        }
        return p->use();
    }
    
    void free(void* addr) {
        auto p = (BlockHeader*)((uchar*)addr - sizeof(BlockHeader));
    }
    
    BlockHeader* new_block(size_t size) {
        auto p = (BlockHeader*)sbrk(size + sizeof(BlockHeader));
        p->init(size);
        tail->prev->append(p);
        return p;
    }
    
    static BlockHeader head_;
    static BlockHeader tail_;
    
    BlockHeader* head;
    BlockHeader* tail;
    BlockHeader* free_head;
    BlockHeader* free_tail;
};

BlockHeader MallocList::head_;
BlockHeader MallocList::tail_;

MallocList malloc_list;

void init_malloc() {
    malloc_list.init();
}

void* kmalloc(size_t size) {
}

void kfree(void* addr) {
}
