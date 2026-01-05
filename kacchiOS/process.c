/* process.c - Process management implementation */
#include "process.h"
#include "memory.h"
#include "scheduler.h"

#define MAX_PROCESSES 8
#define DEFAULT_STACK_SIZE 4096

static process_t process_table[MAX_PROCESSES];
static int next_pid = 1;

static process_t *alloc_pcb(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        if (process_table[i].state == PROC_UNUSED || process_table[i].state == PROC_TERMINATED)
        {
            return &process_table[i];
        }
    }
    return 0;
}

process_t *process_current(void)
{
    return scheduler_current();
}

static void process_bootstrap(process_t *proc)
{
    proc->entry(proc->arg);
    process_exit();
}

static void setup_context(process_t *proc)
{
    uint8_t *top = proc->stack_base + proc->stack_size;
    uint32_t *sp = (uint32_t *)top;

    /* Set up initial stack for process_bootstrap(proc) */
    *(--sp) = (uint32_t)proc; /* Argument for bootstrap */
    *(--sp) = 0;              /* Fake return address */

    proc->ctx.esp = (uint32_t)sp;
    proc->ctx.ebp = (uint32_t)sp;
    proc->ctx.eip = (uint32_t)process_bootstrap;
}

process_t *process_create(process_entry_t entry, void *arg, size_t stack_size)
{
    if (!entry)
    {
        return 0;
    }

    process_t *proc = alloc_pcb();
    if (!proc)
    {
        return 0;
    }

    size_t need = stack_size ? stack_size : DEFAULT_STACK_SIZE;
    uint8_t *stack = (uint8_t *)stack_alloc(need);
    if (!stack)
    {
        return 0;
    }

    proc->pid = next_pid++;
    proc->state = PROC_READY;
    proc->stack_base = stack;
    proc->stack_size = need;
    proc->entry = entry;
    proc->arg = arg;
    proc->next = 0;
    proc->age = 0;
    proc->time_slice = 0;

    setup_context(proc);
    scheduler_add(proc);
    return proc;
}

void process_mark_ready(process_t *proc)
{
    if (!proc)
    {
        return;
    }
    proc->state = PROC_READY;
}

void process_block_current(void)
{
    process_t *self = process_current();
    if (!self)
    {
        return;
    }
    self->state = PROC_BLOCKED;
    scheduler_block_current();
}

void process_exit(void)
{
    process_t *self = process_current();
    if (!self)
    {
        return;
    }

    self->state = PROC_TERMINATED;
    if (self->stack_base)
    {
        stack_free(self->stack_base);
    }
    scheduler_exit_current();
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}

void process_init(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        process_table[i].pid = 0;
        process_table[i].state = PROC_UNUSED;
        process_table[i].stack_base = 0;
        process_table[i].stack_size = 0;
        process_table[i].entry = 0;
        process_table[i].arg = 0;
        process_table[i].next = 0;
    }
}

int process_get_count(void)
{
    return MAX_PROCESSES;
}

process_t *process_get_by_index(int idx)
{
    if (idx < 0 || idx >= MAX_PROCESSES)
        return 0;
    return &process_table[idx];
}
