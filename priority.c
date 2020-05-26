#include "priority.h"

#include <stddef.h>
#include <malloc.h>


int cmp(PriorityQueueElement lhs, PriorityQueueElement rhs);

int find_highest(PriorityQueue *self);

void pq_put(PriorityQueue *self, PriorityQueueElement element) {
    self->elements[self->size++] = element;
}

PriorityQueueElement pq_pop(PriorityQueue *self) {
    // Extract an element with the highest priority
    int extracted_index = find_highest(self);
    PriorityQueueElement extracted_element = self->elements[extracted_index];

    // Replace the extracted element with the last element
    self->elements[extracted_index] = self->elements[self->size - 1];
    self->size--;

    return extracted_element;
}

PriorityQueueElement pq_peek(PriorityQueue *self) {
    int index = find_highest(self);
    return self->elements[index];
}

int cmp(PriorityQueueElement lhs, PriorityQueueElement rhs) {
    if (lhs.timestamp > rhs.timestamp) {
        return 1;
    } else if (lhs.timestamp < rhs.timestamp) {
        return -1;
    } else {
        if (lhs.process_id > rhs.process_id) {
            return 1;
        } else if (lhs.process_id < rhs.process_id) {
            return -1;
        } else {
            return 0;
        }
    }
}

int find_highest(PriorityQueue *self) {
    if (self->size <= 0) {
        return -1;
    }

    int max = 0;
    for (int i = 1; i < self->size; ++i) {
        if (cmp(self->elements[i], self->elements[max]) > 0) {
            max = i;
        }
    }

    return max;
}
