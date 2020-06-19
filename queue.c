//
// Created by antonAdmin on 18.06.2020.
//
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

void initQ()
{
    queue = (lQueue*) malloc(sizeof(lQueue));
    queue -> size = 0;
}


void deleteQ()
{
    if(queue -> size != 0)
    {
        rNode* node = queue -> top;

        for(int i = 0; i < queue->size - 1; i++)
        {
            node = node -> next;
            free(node->prev);
        }

        free(node);
    }

    free(queue);

}

int comp(rNode* t1, rNode* t2) {
    if (t1->time == t2->time){
        if (t1->id > t2->id) {
            return 1;
        } else {
            return 0;
        }
    }
    if (t1->time > t2->time) {
        return 1;
    } else {
        return 0;
    }
}

rNode* top()
{
    rNode* node = (rNode*) malloc(sizeof(rNode));
    node -> id = queue->top->id;
    node -> time = queue->top->time;

    return node;
}

void dTop()
{
    rNode* node = queue->top;

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

void add(local_id id, timestamp_t time)
{
    rNode* node = (rNode*) malloc(sizeof(rNode));

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

    rNode* cur_node = queue -> top;

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

int isQempty()
{
    return queue == 0;
}

size_t qSize()
{
    return queue -> size;
}
