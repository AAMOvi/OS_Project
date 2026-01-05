/* ipc.c - Simple message queue IPC */
#include "ipc.h"
#include "scheduler.h"

static void enqueue_waiter(process_t **list, process_t *proc)
{
    proc->next = *list;
    *list = proc;
}

static process_t *dequeue_waiter(process_t **list)
{
    process_t *p = *list;
    if (p)
    {
        *list = p->next;
        p->next = 0;
    }
    return p;
}

void ipc_init(ipc_queue_t *q)
{
    if (!q)
    {
        return;
    }
    q->head = q->tail = q->count = 0;
    q->waiting_receivers = 0;
    q->waiting_senders = 0;
}

int ipc_send(ipc_queue_t *q, uint32_t value)
{
    if (!q)
    {
        return -1;
    }

    while (q->count == IPC_QUEUE_CAP)
    {
        enqueue_waiter(&q->waiting_senders, scheduler_current());
        process_block_current();
    }

    /* If a receiver is waiting, hand off directly */
    process_t *recv = dequeue_waiter(&q->waiting_receivers);
    if (recv)
    {
        /* Store value in arg; use (value+1) to distinguish 0 from NULL */
        recv->arg = (void *)(uint32_t)(value + 1);
        scheduler_unblock(recv);
        return 0;
    }

    /* Enqueue into buffer */
    q->buf[q->tail] = value;
    q->tail = (q->tail + 1) % IPC_QUEUE_CAP;
    q->count++;
    return 0;
}

int ipc_recv(ipc_queue_t *q, uint32_t *out_value)
{
    if (!q || !out_value)
    {
        return -1;
    }

    while (1)
    {
        if (q->count > 0)
        {
            *out_value = q->buf[q->head];
            q->head = (q->head + 1) % IPC_QUEUE_CAP;
            q->count--;
            break;
        }

        enqueue_waiter(&q->waiting_receivers, scheduler_current());
        process_block_current();

        /* If we were unblocked via direct handoff, arg has (value+1) stored */
        if (scheduler_current()->arg)
        {
            *out_value = (uint32_t)scheduler_current()->arg - 1;
            scheduler_current()->arg = 0;
            break;
        }
    }

    /* Wake a waiting sender if any */
    process_t *sender = dequeue_waiter(&q->waiting_senders);
    if (sender)
    {
        scheduler_unblock(sender);
    }
    return 0;
}
