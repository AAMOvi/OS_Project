/* process.h - Process management */
#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

struct process;

typedef enum
{
    PROC_UNUSED = 0,
    PROC_CURRENT,
    PROC_READY,
    PROC_BLOCKED,
    PROC_TERMINATED
} process_state_t;

typedef void (*process_entry_t)(void *);

typedef struct context
{
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
} context_t;

typedef struct process
{
    int pid;
    process_state_t state;
    context_t ctx;
    uint8_t *stack_base;
    size_t stack_size;
    process_entry_t entry;
    void *arg;
    struct process *next;
    uint32_t age;
    uint32_t time_slice;
} process_t;

void process_init(void);
process_t *process_current(void);
process_t *process_create(process_entry_t entry, void *arg, size_t stack_size);
void process_exit(void);
void process_mark_ready(process_t *proc);
void process_block_current(void);
int process_get_count(void);
process_t *process_get_by_index(int idx);

#endif
