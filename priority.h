#ifndef PRIORITY_H
#define PRIORITY_H

#include "ipc.h"

#define MAX_ELEMENTS 64

typedef struct {
    local_id process_id;
    timestamp_t timestamp;
} PriorityQueueElement;

typedef struct {
    PriorityQueueElement elements[MAX_ELEMENTS];
    int size;
} PriorityQueue;

void pq_put(PriorityQueue *self, PriorityQueueElement element);

PriorityQueueElement pq_pop(PriorityQueue *self);

PriorityQueueElement pq_peek(PriorityQueue *self);

#endif //PRIORITY_H
