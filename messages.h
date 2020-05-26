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

#endif
