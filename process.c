#include "messages.h"
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include "banking.h"
#include "ipc.h"
#include "logging.h"
#include "process.h"
#include "debug.h"
#include "pa2345.h"

void transferOrder(Process *self, Message *mesg);

void initHistory(Process *self, balance_t balance) {
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

void closeOtherPipes(Process *self) {
    for (size_t src = 0; src < self->processes.procesi; src++) {
        for (size_t dest = 0; dest < self->processes.procesi; dest++) {
            if (src != self->id && dest != self->id &&
                src != dest) {
                close(writer[src][dest]);
                close(reader[src][dest]);
            }
            if (src == self->id && dest != self->id) {
                close(reader[src][dest]);
            }
            if (dest == self->id && src != self->id) {
                close(writer[src][dest]);
            }
        }
    }
}

void goParent(Process *self) {

    receiveStartedAll(self);

//    receiveStartedInfo(self);

    bank_robbery(self, self->processes.procesi - 1);

    stopAll(self);

    receiveDoneAll(self);

    receiveBalanceHistories(self);

    print_history(&self->his.vsia_istoria);
}

void goChild(Process *self, balance_t initialBalance) {

    initHistory(self, initialBalance);

    startedAll(self);

    receiveStartedAll(self);

    size_t left = self->processes.deti - 1;

    bool b = false;

    while (!b) {
        Message mesg;
        receive_any(self, &mesg);
        MessageType mesgType = mesg.s_header.s_type;
        switch (mesgType) {
            case STOP:
                b = true;
                break;
            case TRANSFER:
                transferOrder(self, &mesg);
                break;
            case DONE:
                left--;
                break;
            default:
                break;
        }
    }

    doneAll(self);

    while (left > 0) {
        Message message;
        receive_any(self, &message);
        MessageType message_type = message.s_header.s_type;

        switch (message_type) {
            case TRANSFER:
                transferOrder(self, &message);
                break;
            case DONE:
                left--;
                break;
            default:
                break;
        }
    }

    logMsg('d',self);

    historyMaster(self);
}

void transferOrder(Process *self, Message *mesg)  {
    BalanceHistory *history = &self->his.istoria;
    balance_t i = 0;
    TransferOrder *Order = (TransferOrder *) &(mesg->s_payload);
    timestamp_t physicalTime = get_physical_time();

    if (Order->s_src == self->id) {

        send(&myself, Order->s_dst, mesg);

        i = -Order->s_amount;
        logPrintf(log_transfer_out_fmt, get_physical_time(), self->id, Order->s_amount, Order->s_dst);

    } else if (Order->s_dst == self->id) {
        Message msg;


        msg.s_header = (MessageHeader) {
                .s_magic = MESSAGE_MAGIC,
                .s_type = ACK,
                .s_local_time = physicalTime,
                .s_payload_len = 0,
        };
        send(&myself, PARENT_ID, &msg);
        i = +Order->s_amount;

        logPrintf(log_transfer_in_fmt, get_physical_time(), self->id, Order->s_amount, Order->s_src);

    } else {
    }

    if (physicalTime >= history->s_history_len) {
        history->s_history_len = physicalTime + 1;
    }

    for (timestamp_t time = physicalTime; time <= MAX_T; time++) {
        history->s_history[time].s_balance += i;
    }
}

void receveStartedInfo(int *self) {
    self++;
    if(*self == 100 ) return;
    receveStartedInfo(self);
}

void receiveStartedInfo(Process *self){
    int info=0;
    receveStartedInfo(&info);
}

