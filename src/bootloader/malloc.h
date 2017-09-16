#ifndef MALLOC_H
#define MALLOC_H

extern uint32_t* page_directory;
extern uint32_t* page_table;

void* malloc(uint32_t size);

#endif
