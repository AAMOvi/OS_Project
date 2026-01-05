/* scheduler.h - Simple round-robin scheduler */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

void scheduler_init(void);
void scheduler_add(process_t *proc);
void scheduler_start(void);
void scheduler_yield(void);
void scheduler_exit_current(void);
process_t *scheduler_current(void);
void scheduler_set_time_quantum(uint32_t ticks);
void scheduler_block_current(void);
void scheduler_unblock(process_t *proc);
void scheduler_age_ready(void);

#endif
