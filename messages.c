#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "banking.h"
#include "ipc.h"
#include "pa2345.h"
#include "process.h"

void startedAll(proc *self,FILE *logFile) {
    up_time();
    timestamp_t time = get_lamport_time();
    Message msg = {
        .s_header =
            {
                .s_magic = MESSAGE_MAGIC,
                .s_type = STARTED,
                .s_local_time = time,
                .s_payload_len = 0,
            },
    };


    msg.s_header.s_payload_len = strlen(msg.s_payload);
    send_multicast(&myself, &msg);
}

void receiveStartedAll(proc *self,FILE *logFile) {
    for (size_t i = 1; i <= self->processes.deti; i++) {
        Message msg;
        if (i == self->id) {
            continue;
        }
        receive(&myself, i, &msg);
        set_lamport_time(cpmLTime(get_lamport_time(), msg.s_header.s_local_time, 0, 0));
        up_time();
    }
    fprintf(stdout,log_received_all_started_fmt, get_lamport_time(), self->id);
    fprintf(logFile,log_received_all_started_fmt, get_lamport_time(), self->id);
}

void doneAll(proc *self,FILE *logFile) {
    up_time();
    timestamp_t time = get_lamport_time();
    Message msg = {
        .s_header =
            {
                .s_magic = MESSAGE_MAGIC,
                .s_type = DONE,
                .s_local_time = time,
                .s_payload_len = 0,
            },
    };
    msg.s_header.s_payload_len = strlen(msg.s_payload);
    send_multicast(&myself, &msg);
}

void receiveDoneAll(proc *self, FILE *logFile) {
    for (size_t i = 1; i <= self->processes.deti; i++) {
        if (i == self->id) {
            continue;
        }
        Message msg;
        receive(&myself, i, &msg);
        set_lamport_time(cpmLTime(get_lamport_time(), msg.s_header.s_local_time, 0, 0));
        up_time();
    }
        fprintf(logFile, log_received_all_done_fmt, get_lamport_time(), self->id);
        fprintf(stdout, log_received_all_done_fmt, get_lamport_time(), self->id);
}

void stopAll(proc *self) {
    up_time();
    Message msg = {
        .s_header =
            {
                .s_magic = MESSAGE_MAGIC,
                .s_type = STOP,
                .s_payload_len = 0,
                .s_local_time = get_lamport_time(),
            },
    };
    send_multicast(&myself, &msg);
}


void receiveBalanceHistories(proc *self) {
    for (size_t child = 1; child <= self->processes.deti; child++) {
        Message msg;
        receive(&myself, child, &msg);
        set_lamport_time(cpmLTime(get_lamport_time(), msg.s_header.s_local_time, 0, 0));
        up_time();
    }
}
