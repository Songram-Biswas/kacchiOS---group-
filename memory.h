#ifndef MEMORY_H

#define MEMORY_H



#include "types.h"



void memory_init(void);

void* kmalloc(size_t size);

void kfree(void* ptr);

void* alloc_stack(void);

void free_stack(void* stack_top);

void memory_dump(void);



#endif

