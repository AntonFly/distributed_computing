#include "process.h"
#include "debug.h"
#include "priority.h"


int available();

static int forks[MAX_PROCESSES];
static int dirty[MAX_PROCESSES];
static int waiting_for_fork[MAX_PROCESSES];

int request_critical(Process *self_void) {
    for(int i = self_void->id; i < processes; i++)
    {
        if (i != self_void->id) {
            waiting_for_fork[i] = 0;
            dirty[i] = forks[i] = 1;
        } else {
            waiting_for_fork[i] = 2;
            dirty[i] = forks[i] = 2;
        }
    }


    for(int i = 1; i< processes; i++){
        if(forks[i]==0){
            self_void->lamport_time++;

            Message msg = {
                    .s_header =
                            {
                                    .s_magic = MESSAGE_MAGIC,
                                    .s_type = CS_REQUEST,
                                    .s_local_time = get_lamport_time(),
                                    .s_payload_len = 0,
                            },
            };
            send(self_void, i, &msg);
        }
    }

    while (available()==0) {


        Message mesg;
        local_id peer_id = receive_any(self_void, &mesg);
        timestamp_t their_time = mesg.s_header.s_local_time;
        up_time(self_void, their_time);

        switch (mesg.s_header.s_type) {
            case CS_REPLY:
                forks[peer_id]=1;
                break;

            case CS_REQUEST:
                waiting_for_fork[peer_id]=1;
                if(dirty[peer_id]==1){
                    dirty[peer_id]=0;
                    forks[peer_id]=0;
                    mesg.s_header.s_local_time =  increment_and_get_local_time(self_void);
                    mesg.s_header.s_type = CS_REPLY;
                    send(self_void, peer_id, &mesg);
                }
                break;


            case DONE:
                self_void->done_received++;
                break;
        }
    }

    return 0;
}

int release_critical( Process *self_void) {
    for(int i = 1; i< processes; i++){
            Message mesg;
            mesg.s_header.s_local_time =  increment_and_get_local_time(self_void);
            mesg.s_header.s_type = CS_REPLY;
            if(i!=self_void->id) send(self_void, i, &mesg);
    }
    return 0;
}

int release_cs(const void *self) {
    return release_critical((Process *) self);
}
int request_cs(const void *self) {
    return request_critical((Process *) self);
}

int available(){
    for(int i = 1; i < processes; i++){
        if (0 == forks[i] || 1 == dirty[i])
            return 0;
    }
    return 1;
}
