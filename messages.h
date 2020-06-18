#ifndef LAB1_MESSAGES_H
#define LAB1_MESSAGES_H

#include "process.h"

void stopAll(proc *self);

void startedAll(proc *self);

void receiveStartedAll(proc *self);


void doneAll(proc *self);

void receiveDoneAll(proc *self);

void receiveBalanceHistories(proc *self);

void historyMaster(proc *self);

#endif
