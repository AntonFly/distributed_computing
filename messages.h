#ifndef LAB1_MESSAGES_H
#define LAB1_MESSAGES_H

#include "process.h"

void stopAll(Process *self);

void startedAll(Process *self);

void receiveStartedAll(Process *self);


void doneAll(Process *self);

void receiveDoneAll(Process *self);

void receiveBalanceHistories(Process *self);

void historyMaster(Process *self);

#endif
