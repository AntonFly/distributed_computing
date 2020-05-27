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
#include "critical.h"

//void transfer_order(Process *self, Message *mesg);
#define MULTIPLICATION_FACTOR 5
//int max(int lhs, int rhs);

void do_mutex_work(Process *self);

void do_work(Process *self);

const char *MESSAGE_TYPE_STRINGS[] = {
        "STARTED",
        "DONE",
        "ACK",
        "STOP",
        "TRANSFER",
        "BALANCE_HISTORY",
        "CS_REQUEST",
        "CS_REPLY",
        "CS_RELEASE"
};

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

//    bank_robbery(self, processes - 1);

//    stop_all(self);


    receive_done_all(self);

//    receive_balance_histories(self);

    for (size_t i = 1; i <= processes; i++) {
        waitpid(pids[i], NULL, 0);
    }

//    print_history(&self->all_history);
}

void go_child(Process *self) {

//    init_history(self, initial_balance);

    started_all(self);

    receive_started_all(self);


//
//        if (self->mutual_exclusion) {
//            request_cs(self);
//        }
//
//    char str[128];
//    int num_prints = self->id * MULTIPLICATION_FACTOR;
//    for (int i = 1; i <= num_prints; ++i) {
//        memset(str, 0, sizeof(str));
//        sprintf(str, log_loop_operation_fmt, self->id, i, num_prints);
//        print(str);
//    }
//
//    if (self->mutual_exclusion) {
//        release_cs(self);
//    }
//    size_t left = children - 1;

    for (int i = 1; i <= children; ++i) {
        self->is_defer[i] = false;
    }

    // TODO: Send
    do_mutex_work(self);
    done_all(self);

    while (self->done_received < children-1) {
        Message mesg;
        receive_any(self, &mesg);
        up_time(self, mesg.s_header.s_local_time);
        MessageType mesg_type = mesg.s_header.s_type;
        switch (mesg_type) {
            case CS_REQUEST:
                // TODO: Process CS request
                break;
            case CS_RELEASE:
                // TODO: Process CS release
                break;
            case CS_REPLY:
                // TODO: Process CS reply
                break;
            case DONE:
                self->done_received++;
                DEBUG("Received %d out of %lu DONE messages", self->done_received, children - 1);
                break;
            default: {
//                NOTICE("Received unrecognized message type = %d", message_type);
                break;
            }
        }
    }


    log_msg('d',self);

}


timestamp_t get_lamport_time() {
    return myself.lamport_time;
}

timestamp_t get_local_time(Process *self) {
    return self->lamport_time;
}

void up_time(Process *self, timestamp_t their_time) {
    timestamp_t old_time = self->lamport_time;
    if (their_time > self->lamport_time) {
        self->lamport_time = their_time;
    }
    self->lamport_time++;
    DEBUG("Time %d -> %d", old_time, self->lamport_time);
}

timestamp_t increment_and_get_local_time(Process *self) {
    return ++self->lamport_time;
}

void increment_local_time(Process *self) {
    increment_and_get_local_time(self);
}

timestamp_t lift_and_get_local_time(Process *self, timestamp_t their_time) {
    up_time(self, their_time);
    return get_local_time(self);
}

const char *msg_type_to_string(MessageType type) {
    return MESSAGE_TYPE_STRINGS[type];
}

void do_mutex_work(Process *self) {
    if (self->mutual_exclusion) {
        request_cs(self);
    }

    do_work(self);

    if (self->mutual_exclusion) {
        release_cs(self);
    }
}

void do_work(Process *self) {
    char str[128];
    int num = self->id * MULTIPLICATION_FACTOR;

    for (int i = 1; i <= num; ++i) {
        memset(str, 0, sizeof(str));
        sprintf(str, log_loop_operation_fmt, self->id, i, num);
        print(str);
    }
}

