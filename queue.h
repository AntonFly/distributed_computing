//
// Created by antonAdmin on 18.06.2020.
//

#ifndef LAB5_QUEUE_H
#define LAB5_QUEUE_H
#include "ipc.h"

typedef struct request_node request_node;

struct request_node
{
    local_id id;
    timestamp_t time;

    request_node* next;
    request_node* prev;

};

typedef struct lamport_queue
{
    request_node* top;
    request_node* bottom;

    size_t size;

} lamport_queue;

lamport_queue* queue;

void init_queue();

void delete_queue();

int comp(request_node* t1, request_node* t2);

request_node* top();

void del_top();

void insert(local_id id, timestamp_t time);

int is_empty();

size_t queue_size();

#endif //LAB5_QUEUE_H
