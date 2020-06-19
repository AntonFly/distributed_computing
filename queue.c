//
// Created by antonAdmin on 18.06.2020.
//
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

void init_queue()
{
    queue = (lamport_queue*) malloc(sizeof(lamport_queue));
    queue -> size = 0;
}


void delete_queue()
{
    if(queue -> size != 0)
    {
        request_node* node = queue -> top;

        for(int i = 0; i < queue->size - 1; i++)
        {
            node = node -> next;
            free(node->prev);
        }

        free(node);
    }

    free(queue);

}

/*
    1 if t1 have higher priority
*/
int comp(request_node* t1, request_node* t2)
{
    if( t1->time == t2->time)
        return t1->id > t2->id ? 1 : 0;

    return t1->time > t2->time ? 1 : 0;
}

request_node* top()
{
    request_node* node = (request_node*) malloc(sizeof(request_node));
    node -> id = queue->top->id;
    node -> time = queue->top->time;

    return node;
}

void del_top()
{
    request_node* node = queue->top;

    if(queue->size == 1)
    {
        queue -> bottom = NULL;
        queue -> top = NULL;
    }
    else
    {
        node -> next -> prev = NULL;
        queue -> top = node -> next;
        node -> next = NULL;
    }

    queue ->size--;

    free(node);
}

void insert(local_id id, timestamp_t time)
{
    request_node* node = (request_node*) malloc(sizeof(request_node));

    node -> id = id;
    node -> time = time;
    node -> next = NULL;
    node -> prev = NULL;

    if(queue -> size == 0)
    {
        queue -> top = node;
        queue -> bottom = node;
        queue -> size = 1;

        return;
    }

    request_node* cur_node = queue -> top;

    for (int i = 0; i <= queue -> size; i++)
    {
        if(i == queue -> size)
        {
            node -> prev = queue -> bottom;
            queue -> bottom -> next = node;
            queue -> bottom = node;
            break;
        }

        if(comp(node, cur_node))
        {
            node -> next = cur_node;
            if(i == 0)
            {
                queue -> top = node;
                node -> prev = NULL;
            }
            else
            {
                node -> prev = cur_node -> prev;
                cur_node -> prev->next = node;
            }
            cur_node -> prev = node;
            break;

        }
        else cur_node = cur_node -> next;

    }
    queue -> size++;
}

int is_empty()
{
    return queue == 0;
}

size_t queue_size()
{
    return queue -> size;
}
