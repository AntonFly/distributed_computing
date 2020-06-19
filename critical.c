//
// Created by antonAdmin on 18.06.2020.
//

#include "ipc.h"
#include "process.h"
#include "queue.h"

int request_cs(const void * self)
{
    proc* this = (proc*) self;
    up_time();
    timestamp_t rTime = get_lamport_time();
    Message* msg = (Message*) malloc(sizeof(Message));

    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_payload_len = 0;
    msg->s_header.s_type = CS_REQUEST;
    msg->s_header.s_local_time = get_lamport_time();

    send_multicast(this, msg);

    local_id msgFrom;
    int wait = this->processes.deti - 1;

    while(wait > 0)
    {
        if((msgFrom = receive_any(this, msg)) == -1)
            continue;
        set_lamport_time(cpmLTime(get_lamport_time(), msg->s_header.s_local_time, 0, 0));
        up_time();

        switch(msg->s_header.s_type)
        {
            case CS_REQUEST:
                if(rTime > msg->s_header.s_local_time ||
                   (rTime == msg->s_header.s_local_time && this->id < msgFrom))
                {
                    up_time();
                    msg->s_header.s_type = CS_REPLY;
                    msg->s_header.s_local_time = get_lamport_time();

                    send(this, msgFrom, msg);
                }
                else
                {
                    add(msgFrom, msg->s_header.s_local_time);
                }
                break;
            case CS_REPLY:
                wait--;
                break;
            case DONE:
                this -> done++;
                break;
            default:
                continue;
        }
    }


    return 0;
}


int release_cs(const void * self)
{
    proc* this = (proc*) self;

    Message* msg = (Message*) malloc(sizeof(Message));

    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_payload_len = 0;
    msg->s_header.s_type = CS_REPLY;
    while(qSize() >0)
    {
        up_time();
        msg->s_header.s_local_time = get_lamport_time();

        send(this, (top() -> id), msg);

        dTop();

    }

    return 0;
}
