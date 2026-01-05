/* memory.h - Simple heap/stack allocator */
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

void memory_init(void);
void *heap_alloc(size_t size);
void heap_free(void *ptr);
void *stack_alloc(size_t size);
void stack_free(void *ptr);
void memory_get_stats(uint32_t *total_free, uint32_t *largest_block);

#endif
