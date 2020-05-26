#include <stdio.h>
#include "process.h"
#include "debug.h"
#include "priority.h"
//#include "ipc.c"

int request_cs(const Process *self_void) {
    CONVERT_SELF_TO(Process, self_void);

    // Increment Lamport time
    increment_and_get_local_time(self);

    // Add (my_id, lamport_time) to local queue
    PriorityQueue *queue = &self->request_queue;
    pq_put(queue, (PriorityQueueElement) {
        .process_id = self->id,
        .timestamp = get_lamport_time(),
    });

    // Send multicast CS_REQUEST
    {
        Message request_msg = {
            .s_header = {
                .s_magic = MESSAGE_MAGIC,
                .s_type = CS_REQUEST,
                .s_local_time = get_lamport_time(),
                .s_payload_len = 0,
            },
            .s_payload = "",
        };

        send_multicast(self, &request_msg);
    }

    int l_replies = children - 1;
    while (true) {

        if (l_replies == 0 && pq_peek(queue).process_id == self->id) {
            break;
        }

        Message mesg;
        local_id peer = receive_any(self, &mesg);
        up_time(self, mesg.s_header.s_local_time);

        switch (mesg.s_header.s_type) {
            case CS_REPLY:
                l_replies--;
                break;

            case CS_REQUEST:
                pq_put(queue, (PriorityQueueElement) {
                    .process_id = peer,
                    .timestamp = mesg.s_header.s_local_time,
                });

                timestamp_t local_time = increment_and_get_local_time(self);
                Message message = {
                    .s_header = {
                        .s_magic = MESSAGE_MAGIC,
                        .s_local_time = local_time,
                        .s_type = CS_REPLY,
                        .s_payload_len = 0,
                    }
                };
                send(self, peer, &message);

                break;

            case CS_RELEASE:
                pq_pop(queue);
                break;

            case DONE:
                self->done_received++;
                break;
        }
    }

    return 0;
}

int release_cs(const Process *self_void) {
    CONVERT_SELF_TO(Process, self_void);

    // Remove ourselves from the queue
    PriorityQueue *local_queue = &self->request_queue;
    pq_pop(local_queue);

    // Increment lamport time & send multicast CS_RELEASE
    Message message = {
        .s_header = {
            .s_magic = MESSAGE_MAGIC,
            .s_local_time = increment_and_get_local_time(self),
            .s_type = CS_RELEASE,
            .s_payload_len = 0,
        },
        .s_payload = "",
    };

    send_multicast(self, &message);

    return 0;
}
