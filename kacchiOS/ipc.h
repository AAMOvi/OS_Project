/* ipc.h - Simple message queue IPC */
#ifndef IPC_H
#define IPC_H

#include "types.h"
#include "process.h"

#define IPC_QUEUE_CAP 16

typedef struct ipc_queue
{
    uint32_t buf[IPC_QUEUE_CAP];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
    process_t *waiting_receivers;
    process_t *waiting_senders;
} ipc_queue_t;

void ipc_init(ipc_queue_t *q);
int ipc_send(ipc_queue_t *q, uint32_t value);
int ipc_recv(ipc_queue_t *q, uint32_t *out_value);

#endif
