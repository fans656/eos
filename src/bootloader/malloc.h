#ifndef MALLOC_H
#define MALLOC_H

extern uint32_t* page_directory;
extern uint32_t* page_table;

void enable_paging();
void init_memory();

void* malloc(uint32_t size);
void free(void* p);

#endif
