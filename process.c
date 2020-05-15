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

void transfer_order(Process *self, Message *mesg);

void init_history(Process *self, balance_t balance) {
    self->history.s_id = self->id;
    self->history.s_history_len = 1;
    for (timestamp_t time = 0; time <= MAX_T; ++time) {
        self->history.s_history[time] = (BalanceState) {
            .s_balance = balance,
            .s_balance_pending_in = 0,
            .s_time = time,
        };
    }
}

void close_other_pipes(Process *self) {
    for (size_t src = 0; src < processes; src++) {
        for (size_t dest = 0; dest < processes; dest++) {
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

void go_parent(Process *self) {
    receive_started_all(self);

    bank_robbery(self, processes - 1);

    stop_all(self);


    receive_done_all(self);

    receive_balance_histories(self);

    for (size_t i = 1; i <= processes; i++) {
    }

    print_history(&self->all_history);
}

void go_child(Process *self, balance_t initial_balance) {

    init_history(self, initial_balance);

    started_all(self);

    receive_started_all(self);

    bool b = false;
    size_t left = children - 1;

    while (!b) {
        Message mesg;
        receive_any(self, &mesg);
        MessageType mesg_type = mesg.s_header.s_type;
        switch (mesg_type) {
            case STOP:
                b = true;
                break;
            case TRANSFER:
                transfer_order(self, &mesg);
                break;
            case DONE:
                left--;
                break;
            default:
                fprintf(
                        stderr,
                        "Warning: Process %d received unrecognized message type = "
                        "%d\n",
                        self->id, mesg_type);
                break;
        }
    }

    done_all(self);

    while (left > 0) {
        Message message;
        receive_any(self, &message);
        MessageType message_type = message.s_header.s_type;

        switch (message_type) {
            case TRANSFER:
                transfer_order(self, &message);
                break;
            case DONE:
                left--;
                break;
            default:
                fprintf(
                        stderr,
                        "NOTICE: Process %d received unrecognized message type = "
                        "%d\n",
                        self->id, message_type);
                break;
        }
    }

    log_msg('d',self);

    history_master(self);
}

void transfer_order(Process *self, Message *mesg)  {
    TransferOrder *order = (TransferOrder *) &(mesg->s_payload);
    timestamp_t transfer_t = get_physical_time();
    BalanceHistory *history = &self->history;
    balance_t delta = 0;

    if (order->s_src == self->id) {
        delta = -order->s_amount;

        send(&myself, order->s_dst, mesg);

        log_printf(log_transfer_out_fmt, get_physical_time(), self->id, order->s_amount, order->s_dst);

    } else if (order->s_dst == self->id) {
        delta = +order->s_amount;

        Message msg;
        msg.s_header = (MessageHeader) {
                .s_magic = MESSAGE_MAGIC,
                .s_type = ACK,
                .s_local_time = transfer_t,
                .s_payload_len = 0,
        };
        send(&myself, PARENT_ID, &msg);

        log_printf(log_transfer_in_fmt, get_physical_time(), self->id, order->s_amount, order->s_src);

    } else {
    }

    if (transfer_t >= history->s_history_len) {
        history->s_history_len = transfer_t + 1;
    }

    for (timestamp_t time = transfer_t; time <= MAX_T; time++) {
        history->s_history[time].s_balance += delta;
    }
}
