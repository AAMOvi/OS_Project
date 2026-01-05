/* scheduler.c - Round-robin scheduler */
#include "scheduler.h"
#include "serial.h"

static process_t *ready_head = 0;
static process_t *ready_tail = 0;
static process_t *blocked_head = 0;
static process_t *current = 0;
static context_t bootstrap_ctx;
static uint32_t time_quantum_ticks = 1;

extern void context_switch(context_t *old_ctx, context_t *new_ctx);

static process_t *pop_ready(void)
{
    process_t *p = ready_head;
    if (p)
    {
        ready_head = p->next;
        if (!ready_head)
        {
            ready_tail = 0;
        }
        p->next = 0;
    }
    return p;
}

static void scheduler_age_ready_internal(void)
{
    process_t *p = ready_head;
    while (p)
    {
        if (p->age < 0xFFFFFFFF)
        {
            p->age++;
        }
        p = p->next;
    }
}

static void place_ready_with_aging(process_t *proc)
{
    if (!proc)
    {
        return;
    }
    proc->state = PROC_READY;
    proc->time_slice = time_quantum_ticks;

    const uint32_t AGE_THRESHOLD = 3;

    if (!ready_head)
    {
        ready_head = ready_tail = proc;
        proc->next = 0;
        return;
    }

    if (proc->age >= AGE_THRESHOLD)
    {
        proc->next = ready_head;
        ready_head = proc;
        if (!ready_tail)
        {
            ready_tail = proc;
        }
        proc->age = 0;
        return;
    }

    proc->next = 0;
    ready_tail->next = proc;
    ready_tail = proc;
}

void scheduler_add(process_t *proc)
{
    if (!proc)
    {
        return;
    }
    proc->age = 0;
    place_ready_with_aging(proc);
}

void scheduler_age_ready(void)
{
    scheduler_age_ready_internal();
}

process_t *scheduler_current(void)
{
    return current;
}

void scheduler_init(void)
{
    ready_head = ready_tail = 0;
    blocked_head = 0;
    current = 0;
    time_quantum_ticks = 1;
}

void scheduler_set_time_quantum(uint32_t ticks)
{
    time_quantum_ticks = ticks ? ticks : 1;
}

void scheduler_start(void)
{
    process_t *next = pop_ready();
    if (!next)
    {
        return;
    }
    current = next;
    current->state = PROC_CURRENT;
    context_switch(&bootstrap_ctx, &current->ctx);
}

void scheduler_yield(void)
{
    process_t *prev = current;
    scheduler_age_ready_internal();
    process_t *next = pop_ready();

    if (!next)
    {
        return;
    }

    if (prev && prev->state == PROC_CURRENT)
    {
        prev->age = 0;
        place_ready_with_aging(prev);
    }

    next->state = PROC_CURRENT;
    current = next;
    context_switch(&prev->ctx, &next->ctx);
}

void scheduler_exit_current(void)
{
    process_t *prev = current;
    process_t *next = pop_ready();

    current = next;
    if (next)
    {
        next->state = PROC_CURRENT;
        context_switch(&prev->ctx, &next->ctx);
    }

    /* No runnable processes remain */
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}

void scheduler_block_current(void)
{
    process_t *self = current;
    if (!self)
    {
        return;
    }
    self->state = PROC_BLOCKED;
    self->age = 0;
    self->next = blocked_head;
    blocked_head = self;

    process_t *next = pop_ready();
    if (!next)
    {
        /* No ready process; system deadlock or all blocked */
        for (;;)
        {
            __asm__ volatile("hlt");
        }
    }

    next->state = PROC_CURRENT;
    current = next;
    context_switch(&self->ctx, &next->ctx);
    /* When unblocked, execution resumes here */
}

void scheduler_unblock(process_t *proc)
{
    if (!proc || proc->state != PROC_BLOCKED)
    {
        return;
    }

    process_t **pp = &blocked_head;
    while (*pp)
    {
        if (*pp == proc)
        {
            *pp = proc->next;
            break;
        }
        pp = &(*pp)->next;
    }

    proc->next = 0;
    proc->age = 0;
    place_ready_with_aging(proc);
}
