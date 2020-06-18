#ifndef LAB1_MESSAGES_H
#define LAB1_MESSAGES_H

#include "process.h"

void stopAll(proc *self);

void startedAll(proc *self,FILE *logFile);

void receiveStartedAll(proc *self,FILE *logFile);

void doneAll(proc *self,FILE *logFile);

void receiveDoneAll(proc *self,FILE *logFil);

void receiveBalanceHistories(proc *self);


#endif
