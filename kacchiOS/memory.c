/* memory.c - Simple heap and stack allocator */
#include "memory.h"
#include "types.h"

#define HEAP_SIZE (64 * 1024)
#define ALIGNMENT 16

typedef struct mem_block
{
    uint32_t size;          /* Size of the block payload */
    uint32_t free;          /* 1 if free, 0 if used */
    struct mem_block *next; /* Next block in the free list */
} mem_block_t;

static uint8_t heap_area[HEAP_SIZE];
static mem_block_t *free_list = 0;

static uint32_t align_up(uint32_t value)
{
    uint32_t rem = value % ALIGNMENT;
    return rem ? (value + ALIGNMENT - rem) : value;
}

void memory_init(void)
{
    uint32_t base = (uint32_t)heap_area;
    uint32_t aligned = align_up(base);
    uint32_t offset = aligned - base;
    free_list = (mem_block_t *)(heap_area + offset);
    free_list->size = HEAP_SIZE - offset - sizeof(mem_block_t);
    free_list->free = 1;
    free_list->next = 0;
}

static void split_block(mem_block_t *block, uint32_t size)
{
    if (block->size >= size + sizeof(mem_block_t) + ALIGNMENT)
    {
        uint8_t *next_addr = (uint8_t *)block + sizeof(mem_block_t) + size;
        mem_block_t *next = (mem_block_t *)next_addr;
        next->size = block->size - size - sizeof(mem_block_t);
        next->free = 1;
        next->next = block->next;
        block->size = size;
        block->next = next;
    }
}

void *heap_alloc(size_t size)
{
    if (!size)
    {
        return 0;
    }

    uint32_t need = align_up((uint32_t)size);
    mem_block_t *cur = free_list;

    while (cur)
    {
        if (cur->free && cur->size >= need)
        {
            split_block(cur, need);
            cur->free = 0;
            return (uint8_t *)cur + sizeof(mem_block_t);
        }
        cur = cur->next;
    }
    return 0;
}

static void coalesce(void)
{
    mem_block_t *cur = free_list;
    while (cur && cur->next)
    {
        uint8_t *cur_end = (uint8_t *)cur + sizeof(mem_block_t) + cur->size;
        if (cur->free && cur->next->free && cur_end == (uint8_t *)cur->next)
        {
            cur->size += sizeof(mem_block_t) + cur->next->size;
            cur->next = cur->next->next;
        }
        else
        {
            cur = cur->next;
        }
    }
}

void heap_free(void *ptr)
{
    if (!ptr)
    {
        return;
    }
    mem_block_t *block = (mem_block_t *)((uint8_t *)ptr - sizeof(mem_block_t));
    block->free = 1;
    coalesce();
}

void *stack_alloc(size_t size)
{
    /* Stack grows downward; we still carve from heap and return base */
    return heap_alloc(size);
}

void stack_free(void *ptr)
{
    heap_free(ptr);
}

void memory_get_stats(uint32_t *total_free, uint32_t *largest_block)
{
    uint32_t total = 0;
    uint32_t largest = 0;
    mem_block_t *cur = free_list;
    while (cur)
    {
        if (cur->free)
        {
            total += cur->size;
            if (cur->size > largest)
            {
                largest = cur->size;
            }
        }
        cur = cur->next;
    }
    if (total_free)
        *total_free = total;
    if (largest_block)
        *largest_block = largest;
}
