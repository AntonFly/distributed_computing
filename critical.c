//
// Created by antonAdmin on 18.06.2020.
//

#include <stdlib.h>

#include "pa2345.h"
#include "ipc.h"
#include "process.h"
#include "queue.h"

int request_cs(const void * self)
{
    proc* this = (proc*) self;
    up_time();
    timestamp_t request_time = get_lamport_time();
    Message* msg = (Message*) malloc(sizeof(Message));

    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_payload_len = 0;
    msg->s_header.s_type = CS_REQUEST;
    msg->s_header.s_local_time = get_lamport_time();

    send_multicast(this, msg);

    local_id msg_id;
    int rep_to_wait = this->processes.deti - 1;
//    printf("request time %d from %d\n",request_time, this->id);

    while(rep_to_wait > 0)
    {
//        printf("rrece  %d\n", 5);
        if((msg_id = receive_any(this, msg)) == -1)
            continue;
//            printf("get massage from %d with header %d\n",msg_id,  msg->s_header.s_type);
        set_lamport_time(compare_lamport_times(get_lamport_time(), msg->s_header.s_local_time, 0, 0));
        up_time();

        switch(msg -> s_header.s_type)
        {
            case CS_REQUEST:
//                     printf("%d from %d\n", this->id, msg_id);
                if(request_time > msg->s_header.s_local_time ||
                   (request_time == msg->s_header.s_local_time && this->id < msg_id))
                {
                    up_time();
//                    printf("%d request\n",get_lamport_time());
                    msg->s_header.s_type = CS_REPLY;
                    msg->s_header.s_local_time = get_lamport_time();

                    send(this, msg_id, msg);
                }
                else
                {
                    insert(msg_id, msg->s_header.s_local_time);
                }
                break;
            case CS_REPLY:
                rep_to_wait--;
                break;
            case DONE:
//                printf("\nme: %d, from %d\n", this->id, msg_id);
                this -> done++;
                break;
            default:
                continue;
        }
    }

    free(msg);

    return 0;
}


int release_cs(const void * self)
{
    proc* current_node = (proc*) self;

    Message* msg = (Message*) malloc(sizeof(Message));

    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_payload_len = 0;
    msg->s_header.s_type = CS_REPLY;
// printf("%d to %d size: %zu\n", current_node->id, top()->id, queue_size() );
    while(queue_size() >0)
    {
        up_time();
        msg->s_header.s_local_time = get_lamport_time();

        send(current_node, (top() -> id), msg);

        del_top();

    }

    free(msg);
    return 0;
}
