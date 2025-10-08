#include "msg.h"
#include "queue.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static pcb_t* find_shortest_job(queue_t *rq) {
    if (!rq || !rq->head) return NULL;

    queue_elem_t *curr = rq->head;
    queue_elem_t *shortest_elem = curr;
    queue_elem_t *prev_shortest = NULL;
    queue_elem_t *prev = NULL;

    // resolvido o problema q tinh a aquu
    while (curr != NULL) {
        if (curr->pcb->time_ms < shortest_elem->pcb->time_ms) {
            shortest_elem = curr;
            prev_shortest = prev;
        }
        prev = curr;
        curr = curr->next;
    }


    if (prev_shortest) {
        prev_shortest->next = shortest_elem->next;
    } else {
        // resolvido o outro bug estupido aqui
        rq->head = shortest_elem->next;
    }


    if (shortest_elem == rq->tail) {
        rq->tail = prev_shortest;
    }

    pcb_t *shortest = shortest_elem->pcb;
    free(shortest_elem);
    return shortest;
}

void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {
    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;


        printf("SJF: PID=%d, elapsed=%u/%u ms\n",  // esse bagulho é util pra mostrar q caralho q ta acontecendo
               (*cpu_task)->pid,
               (*cpu_task)->ellapsed_time_ms,
               (*cpu_task)->time_ms);

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };

            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }

            printf("SJF: PID=%d FINISHED at time %u ms\n", (*cpu_task)->pid, current_time_ms);
            free(*cpu_task);
            *cpu_task = NULL;
        }
    }


    if (*cpu_task == NULL) {
        *cpu_task = find_shortest_job(rq);
        if (*cpu_task) {
            printf("SJF: Selected PID=%d with time=%u ms\n",
                   (*cpu_task)->pid, (*cpu_task)->time_ms);
        }
    }
}