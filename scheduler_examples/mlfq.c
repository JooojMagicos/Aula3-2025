#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mlfq.h"
#include "msg.h"

void mlfq_scheduler(uint32_t current_time_ms, mlfq_ready_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {

        uint32_t slice_elapsed_ms = current_time_ms - (*cpu_task)->slice_start_ms;
        uint8_t level = (*cpu_task)->level;

        printf("DEBUG: PID=%d, level=%d, slice_elapsed=%u/%u, total_elapsed=%u/%u\n",
               (*cpu_task)->pid, level, slice_elapsed_ms, rq->quanta[level],
               (*cpu_task)->ellapsed_time_ms, (*cpu_task)->time_ms);


        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };

            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            printf("DEBUG: PID=%d finished execution\n", (*cpu_task)->pid);
            *cpu_task = NULL;
            return;
        }


        if (slice_elapsed_ms >= rq->quanta[level]) {

            printf("DEBUG: PID=%d preempted from level %d\n", (*cpu_task)->pid, level);

            if (level < NUM_MLFQ_LEVELS - 1) {
                (*cpu_task)->level = level + 1;
                printf("DEBUG: PID=%d demoted to level %d\n", (*cpu_task)->pid, (*cpu_task)->level);
            }

            enqueue_pcb(&rq->levels[(*cpu_task)->level], *cpu_task);
            *cpu_task = NULL;
            return;
        }

        // CORREÇÃO: Incrementar tempo decorrido apenas se ainda não terminou
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
    }

    // CPU idle → escolher próximo
    if (!*cpu_task) {
        printf("DEBUG: CPU idle, looking for next task...\n");
        for (int l = 0; l < NUM_MLFQ_LEVELS; l++) {
            if (rq->levels[l].head != NULL) {
                *cpu_task = dequeue_pcb(&rq->levels[l]);
                if (*cpu_task) {
                    (*cpu_task)->slice_start_ms = current_time_ms;
                    printf("DEBUG: Selected PID=%d from level %d\n", (*cpu_task)->pid, l);
                    break;
                }
            }
        }
    }
}