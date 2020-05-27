#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "banking.h"
#include "ipc.h"
#include "logging.h"
#include "pa2345.h"
#include "process.h"
#include "debug.h"

void started_all(Process *self) {
    self->lamport_time++;

    Message msg = {
            .s_header =
                    {
                            .s_magic = MESSAGE_MAGIC,
                            .s_type = STARTED,
                            .s_local_time = get_lamport_time(),
                            .s_payload_len = 0,
                    },
    };

    timestamp_t time = get_lamport_time();
    printf_log_msg(
        &msg, log_started_fmt, time, self->id, getpid(),
        getppid(),
        self->history.s_history[time].s_balance);

    msg.s_header.s_payload_len = strlen(msg.s_payload);
    send_multicast(&myself, &msg);
}

void receive_started_all(Process *self) {
    for (size_t i = 1; i <= children; i++) {
        Message msg;
        if (i == self->id) {
            continue;
        }
        receive(&myself, i, &msg);
        if (msg.s_header.s_type != STARTED) {
//            fprintf(stderr, "Process %d expect message of type %d (STARTED), got message type %d\n", self->id,
//                    STARTED, msg.s_header.s_type);
            ERROR("Expected message of type %s, got message type %s",
                msg_type_to_string(STARTED), msg_type_to_string(msg.s_header.s_type));
        }
        up_time(self, msg.s_header.s_local_time);
    }
    log_msg('a',self);
}

void done_all(Process *self) {
    self->lamport_time++;
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
    printf_log_msg(&msg, log_done_fmt, time, self->id,
                          self->history.s_history[time].s_balance);
    msg.s_header.s_payload_len = strlen(msg.s_payload);
    send_multicast(&myself, &msg);
}

void receive_done_all(Process *self) {
    for (size_t i = 1; i <= children; i++) {
        if (i == self->id) {
            continue;
        }
        Message msg;
        receive(&myself, i, &msg);
        if (msg.s_header.s_type != DONE) {
            NOTICE("Expected message of type %s, got %s\n",
                   msg_type_to_string(DONE), msg_type_to_string(msg.s_header.s_type));
        }
        up_time(self, msg.s_header.s_local_time);
    }
    log_msg('d',self);
}

Message construct_message_with_type(MessageType message_type) {
    Message request_msg = {
            .s_header = {
                    .s_magic = MESSAGE_MAGIC,
                    .s_type = message_type,
                    .s_local_time = get_lamport_time(),
                    .s_payload_len = 0,
            },
            .s_payload = "",
    };
    return request_msg;
}
