#ifndef LAB1_MESSAGES_H
#define LAB1_MESSAGES_H

#include "process.h"

//void stop_all(Process *self);

void started_all(Process *self);

void receive_started_all(Process *self);


void done_all(Process *self);

void receive_done_all(Process *self);

//void receive_balance_histories(Process *self);

//void history_master(Process *self);
Message construct_message_with_type(MessageType message_type);
#endif
