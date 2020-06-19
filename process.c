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



void changeBalance(proc *self, Message *mesg);

//void initHistory(proc *self, balance_t balance) {
//    self->his.istoria.s_id = self->id;
//    self->his.istoria.s_history_len = 1;
//    for (timestamp_t time = 0; time <= MAX_T; ++time) {
//        self->his.istoria.s_history[time] = (BalanceState) {
//            .s_balance = balance,
//            .s_balance_pending_in = 0,
//            .s_time = time,
//        };
//    }
//}

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
//    size_t left = self->processes.deti - 1;

    char tmp[64];
    if(m==1)
    {
        init_queue();
        request_cs(self);
    }



    for( int i = 1; i <= self->id * 5; i++)
    {
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, log_loop_operation_fmt, self -> id, i, self->id * 5);

        print(tmp);
    }



    if(m==1)
    {
        release_cs(self);

        delete_queue();
    }

//    while (true) {
//        Message mesg;
//        receive_any(self, &mesg);
//        set_lamport_time(compare_lamport_times(get_lamport_time(),mesg.s_header.s_local_time,0,0));
//        up_time();
//        MessageType mesgType = mesg.s_header.s_type;
//        if(mesgType== STOP){
//            break;
//        } else if(mesgType== TRANSFER){
//            changeBalance(self, &mesg);
//        } else if(mesgType == DONE){
//            left--;
//        }
//
//    }

    doneAll(self,logFile);

    while (self->done!=self->processes.deti-1) {
        Message message;
        receive_any(self, &message);
        set_lamport_time(compare_lamport_times(get_lamport_time(),message.s_header.s_local_time,0,0));
        up_time();
        MessageType messageType = message.s_header.s_type;
//        if(messageType == TRANSFER){
//            changeBalance(self, &message);
//        } else
        if (messageType ==DONE ){
//            printf("\nme: %d, from %d\n", self->id, msg_id);
            self->done++;
        }
    }
//    fprintf(logFile,log_done_fmt, get_lamport_time(), self->id,self->his.istoria.s_history[self->his.istoria.s_history_len].s_balance);
//    fprintf(stdout,log_done_fmt, get_lamport_time(), self->id,self->his.istoria.s_history[self->his.istoria.s_history_len].s_balance);

//    self->his.istoria.s_history_len = get_lamport_time() + 1;
//    size_t size_of_history = sizeof(local_id) +
//                             sizeof(uint8_t) +
//                             self->his.istoria.s_history_len * sizeof(BalanceState);

    up_time();
    Message msg = {
            .s_header = {
                    .s_magic = MESSAGE_MAGIC,
                    .s_type = BALANCE_HISTORY,
                    .s_local_time = get_lamport_time(),
//                    .s_payload_len = size_of_history,
            }
    };
//    memcpy(&msg.s_payload, &self->his.istoria, size_of_history);
    send(self, PARENT_ID, &msg);
}

void changeBalance(proc *self, Message *mesg)  {

//    balance_t i = 0;
    TransferOrder *Order = (TransferOrder *) &(mesg->s_payload);
    if (Order->s_dst != self->id) {
//        i = -Order->s_amount;
        up_time();
//        updateHistory(i,get_lamport_time(),self);
        mesg->s_header.s_local_time=get_lamport_time();
        send(&myself, Order->s_dst, mesg);
        fprintf(stdout,log_transfer_out_fmt, get_lamport_time(), self->id, Order->s_amount, Order->s_dst);
    } else {
//        i = +Order->s_amount;
//        updateHistory(i,get_lamport_time(),self);
        up_time();
        Message msg;
        msg.s_header = (MessageHeader) {
                .s_magic = MESSAGE_MAGIC,
                .s_type = ACK,
                .s_local_time = get_lamport_time(),
                .s_payload_len = 0,
        };
        send(&myself, PARENT_ID, &msg);
        fprintf(stdout,log_transfer_in_fmt, get_lamport_time(), self->id, Order->s_amount, Order->s_src);

    }


//    updateHistory(i,get_lamport_time(),self);//govno
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

timestamp_t compare_lamport_times(timestamp_t t1, timestamp_t t2, local_id i1, local_id i2)
{
    if (t1 == t2)
        return i1 > i2 ? t1 : t2;

    return t1 > t2 ? t1 : t2;
}

//void updateHistory(balance_t balanse,timestamp_t time, proc *self){
//    BalanceHistory *history = &self->his.istoria;
//    if (time >= history->s_history_len) {
//        history->s_history_len = time + 1;
//    }
//
//    while (time <= MAX_T){
//        history->s_history[time].s_balance += balanse;
//        time++;
//    }
//}
