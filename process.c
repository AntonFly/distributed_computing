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

void changeBalance(proc *self, Message *mesg);

void initHistory(proc *self, balance_t balance) {
    self->his.istoria.s_id = self->id;
    self->his.istoria.s_history_len = 1;
    for (timestamp_t time = 0; time <= MAX_T; ++time) {
        self->his.istoria.s_history[time] = (BalanceState) {
            .s_balance = balance,
            .s_balance_pending_in = 0,
            .s_time = time,
        };
    }
}

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


void goChild(proc *self, balance_t initialBalance, FILE *logFile) {

    initHistory(self, initialBalance);

    startedAll(self, logFile);

    receiveStartedAll(self, logFile);

    size_t left = self->processes.deti - 1;


    while (true) {
        Message mesg;
        receive_any(self, &mesg);
        MessageType mesgType = mesg.s_header.s_type;
        if(mesgType== STOP){
            break;
        } else if(mesgType== TRANSFER){
            changeBalance(self, &mesg);
        } else if(mesgType == DONE){
            left--;
        }

    }

    doneAll(self,logFile);

    while (left > 0) {
        Message message;
        receive_any(self, &message);
        MessageType messageType = message.s_header.s_type;
        if(messageType == TRANSFER){
            changeBalance(self, &message);
        } else if (messageType ==DONE ){
            left--;
        }
    }
    fprintf(logFile,log_done_fmt, get_physical_time(), self->id,self->his.istoria.s_history[self->his.istoria.s_history_len].s_balance);
    fprintf(stdout,log_done_fmt, get_physical_time(), self->id,self->his.istoria.s_history[self->his.istoria.s_history_len].s_balance);

    self->his.istoria.s_history_len = get_physical_time() + 1;
    size_t size_of_history = sizeof(local_id) +
                             sizeof(uint8_t) +
                             self->his.istoria.s_history_len * sizeof(BalanceState);

    Message msg = {
            .s_header = {
                    .s_magic = MESSAGE_MAGIC,
                    .s_type = BALANCE_HISTORY,
                    .s_local_time = get_physical_time(),
                    .s_payload_len = size_of_history,
            }
    };
    memcpy(&msg.s_payload, &self->his.istoria, size_of_history);
    send(self, PARENT_ID, &msg);
}

void changeBalance(proc *self, Message *mesg)  {
    BalanceHistory *history = &self->his.istoria;
    balance_t i = 0;
    TransferOrder *Order = (TransferOrder *) &(mesg->s_payload);
    timestamp_t physicalTime = get_physical_time();

    if (Order->s_dst != self->id) {
        i = -Order->s_amount;
        send(&myself, Order->s_dst, mesg);
        fprintf(stdout,log_transfer_out_fmt, get_physical_time(), self->id, Order->s_amount, Order->s_dst);
    } else {
        i = +Order->s_amount;
        Message msg;
        msg.s_header = (MessageHeader) {
                .s_magic = MESSAGE_MAGIC,
                .s_type = ACK,
                .s_local_time = physicalTime,
                .s_payload_len = 0,
        };
        send(&myself, PARENT_ID, &msg);
        fprintf(stdout,log_transfer_in_fmt, get_physical_time(), self->id, Order->s_amount, Order->s_src);

    }

    if (physicalTime >= history->s_history_len) {
        history->s_history_len = physicalTime + 1;
    }
    timestamp_t time = physicalTime;
    while (time <= MAX_T){
        history->s_history[time].s_balance += i;
        time++;
    }
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

