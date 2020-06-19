#include "messages.h"
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include "banking.h"
#include "ipc.h"
#include "process.h"
#include "pa2345.h"
#include "queue.h"

static timestamp_t lTime = 0;

void do_mutex_work(proc *self,int m);

void do_work(proc *self);


void closePipes(proc *self) {
    for (size_t src = 0; src < self->processes.procesi; src++) {
        for (size_t dest = 0; dest < self->processes.procesi; dest++) {
            if (dest == self->id && src != self->id) {
                close(writer[src][dest]);
            }
            if (src == self->id && dest != self->id) {
                close(reader[src][dest]);
            }
            if (src != self->id && dest != self->id &&
                src != dest) {
                close(writer[src][dest]);
                close(reader[src][dest]);
            }
        }
    }
}


void goChild(proc *self, FILE *logFile,int m) {

    startedAll(self, logFile);
    receiveStartedAll(self, logFile);

//    char tmp[64];
    do_mutex_work(self,m);
//    if(m==1)
//    {
//        initQ();
//        request_cs(self);
//    }
//
//
//
//    for( int i = 1; i <= self->id * 5; i++)
//    {
//        memset(tmp, 0, sizeof(tmp));
//        sprintf(tmp, log_loop_operation_fmt, self -> id, i, self->id * 5);
////        print(tmp);
//        print(tmp);
//
//    }
//    memset(tmp, 0, sizeof(tmp));
//    sprintf(tmp, log_loop_operation_fmt, 82 ,5,  5);
////        print(tmp);
//    print(tmp);
//
//
//
//
//    if(m==1)
//    {
//        release_cs(self);
//
//        deleteQ();
//    }


    doneAll(self,logFile);

    while (self->done!=self->processes.deti-1) {
        Message message;
        receive_any(self, &message);
        set_lamport_time(cpmLTime(get_lamport_time(), message.s_header.s_local_time, 0, 0));
        up_time();
        MessageType messageType = message.s_header.s_type;
        if (messageType ==DONE ){
            self->done++;
        }
    }
        fprintf(logFile, log_received_all_done_fmt, get_lamport_time(), self->id);
        fprintf(stdout, log_received_all_done_fmt, get_lamport_time(), self->id);

    up_time();
    Message msg = {
            .s_header = {
                    .s_magic = MESSAGE_MAGIC,
                    .s_type = BALANCE_HISTORY,
                    .s_local_time = get_lamport_time(),
            }
    };
    send(self, PARENT_ID, &msg);
}




void receveStartedInfo(int *self) {
    self++;
    if(*self == 100 ) return;
    receveStartedInfo(self);
}

void receiveStartedInfo(proc *self){
    int info=0;
    receveStartedInfo(&info);
}

timestamp_t get_lamport_time() { return lTime; }

void set_lamport_time(timestamp_t new_time) { lTime = new_time; }

void up_time() { lTime += 1; }

timestamp_t cpmLTime(timestamp_t t1, timestamp_t t2, local_id i1, local_id i2)
{
    if (t1 == t2)
        return i1 > i2 ? t1 : t2;

    return t1 > t2 ? t1 : t2;
}


void do_mutex_work(proc *self,int m) {
    if (m==1) {
        initQ();
        request_cs(self);
    }

    do_work(self);

    if (m==1) {
        release_cs(self);
        deleteQ();
    }
}

void do_work(proc *self) {
    char str[128];
    int num = self->id * 5;

    for (int i = 1; i <= num; ++i) {
        memset(str, 0, sizeof(str));
        sprintf(str, log_loop_operation_fmt, self->id, i, num);
        print(str);
    }
}
