//
// Created by antonAdmin on 18.06.2020.
//

#ifndef LAB5_QUEUE_H
#define LAB5_QUEUE_H
#include "ipc.h"

typedef struct rNode rNode;

struct rNode
{
    local_id id;
    timestamp_t time;

    rNode* next;
    rNode* prev;

};

typedef struct lQueue
{
    rNode* top;
    rNode* bottom;

    size_t size;

} lQueue;

lQueue* queue;

void initQ();

void deleteQ();

int comp(rNode* t1, rNode* t2);

rNode* top();

void dTop();

void add(local_id id, timestamp_t time);

int isQempty();

size_t qSize();

#endif //LAB5_QUEUE_H
