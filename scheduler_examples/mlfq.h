#ifndef MLFQ_H
#define MLFQ_H

#include "queue.h"


#define NUM_MLFQ_LEVELS 3

typedef struct {
    queue_t levels[NUM_MLFQ_LEVELS];
    uint32_t quanta[NUM_MLFQ_LEVELS];
} mlfq_ready_t;

void mlfq_scheduler(uint32_t current_time_ms, mlfq_ready_t *rq, pcb_t **cpu_task);

#endif // MLFQ_H
