#include <stdio.h>
#include "process.h"
#include "debug.h"
#include "priority.h"
#include "messages.h"

void defer_reply(Process *self, local_id peer);

void send_replies(Process *self);

int request_critical(Process *self_void) {

    timestamp_t const our_time = increment_and_get_local_time(self_void);

    {
        Message request_msg = construct_message_with_type(CS_REQUEST);
        send_multicast(self_void, &request_msg);
    }

    int l_replies = children - 1;
    while (l_replies > 0) {

//        if (l_replies == 0 && pq_peek(queue).process_id == self->id) {
//            break;
//        }

        Message mesg;
        local_id peer_id = receive_any(self_void, &mesg);
        timestamp_t their_time = mesg.s_header.s_local_time;
        up_time(self_void, their_time);

        switch (mesg.s_header.s_type) {
            case CS_REPLY:
                l_replies--;
                break;

            case CS_REQUEST:
                if (our_time > their_time || (our_time == their_time && self_void->id > peer_id)) {
                    self_void->is_defer[peer_id] = false;
                    increment_and_get_local_time(self_void);
                    Message message = construct_message_with_type(CS_REPLY);
                    send(self_void, peer_id, &message);
                } else {
                    defer_reply(self_void, peer_id);
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
    send_replies(self_void);
    return 0;
}

int release_cs(const void *self) {
    return release_critical((Process *) self);
}
int request_cs(const void *self) {
    return request_critical((Process *) self);
}

void defer_reply(Process *self, local_id peer) {
    DEBUG("Deferring CS_REPLY to process #%d", peer);
    self->is_defer[peer] = true;
}
void send_replies(Process *self) {
    DEBUG("Sending deferred replies");

    for (local_id peer = 1; peer <= children; ++peer) {
        if (peer != self->id && self->is_defer[peer]) {
            DEBUG("Sending deferred CS_REPLY to %d", peer);
            increment_and_get_local_time(self);
            Message message = construct_message_with_type(CS_REPLY);
            send(self, peer, &message);
            self->is_defer[peer] = false;
        }
    }
}
