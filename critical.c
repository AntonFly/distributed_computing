//
// Created by antonAdmin on 18.06.2020.
//

#include "ipc.h"
#include "process.h"

int available(const void * self);

static int forks[MAX_PROCESSES];
static int dirty[MAX_PROCESSES];
static int waiting_for_fork[MAX_PROCESSES];

int request_cs(const void * self)
{

    proc* this = (proc*) self;
    int i =this->id;
    while(i<this->processes.procesi)
    {
        if (i != this->id) {
            waiting_for_fork[i] = 0;
            dirty[i] = forks[i] = 1;
        } else {
            waiting_for_fork[i] = 2;
            dirty[i] = forks[i] = 2;
        }
        i++;
    }
    Message msg = {
            .s_header =
                    {
                            .s_magic = MESSAGE_MAGIC,
                            .s_type = CS_REQUEST,
                            .s_payload_len = 0,
                    },
    };
    int j =1;
    while(j<this->processes.procesi){
        if(forks[j]==0){
            up_time();

            msg.s_header.s_local_time = get_lamport_time();
            send(this, j, &msg);
            waiting_for_fork[j]=0;
        }
        j++;
    }

    local_id msgFrom;

    while(available(this) == 0)
    {
        if((msgFrom = receive_any(this, &msg)) == -1)
            continue;
        set_lamport_time(cpmLTime(get_lamport_time(), msg.s_header.s_local_time, 0, 0));
        up_time();

        switch(msg.s_header.s_type)
        {
            case CS_REQUEST:
                waiting_for_fork[msgFrom]=1;
                if(dirty[msgFrom]==1){
                    dirty[msgFrom]=0;
                    forks[msgFrom]=0;
                    up_time();
                    msg.s_header.s_local_time =  get_lamport_time();
                    msg.s_header.s_type = CS_REPLY;
                    send(this, msgFrom, &msg);
                }
                break;
            case CS_REPLY:
                forks[msgFrom]=1;
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
    Message msg = {
            .s_header =
                    {
                            .s_magic = MESSAGE_MAGIC,
                            .s_type = CS_REPLY,
                            .s_payload_len = 0,
                    },
    };
    for(int i = 1; i< this->processes.procesi; i++){
        up_time();
        msg.s_header.s_local_time =  get_lamport_time();
        if(i!=this->id) send(this, i, &msg);
    }
    return 0;
}

int available(const void * self){
    proc* this = (proc*) self;
    for(int i = 1; i < this->processes.procesi; i++){
        if (0 == forks[i] || 1 == dirty[i])
            return 0;
    }
    return 1;
}
