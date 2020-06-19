#ifndef LAB1_PROCESS_H
#define LAB1_PROCESS_H

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include "banking.h"

#include "ipc.h"

typedef struct {
    size_t procesi;
    size_t deti;
} mount_p;


typedef struct {
    BalanceHistory istoria;
    AllHistory vsiaIstoria;
} history;

typedef struct {
    local_id id;
    history his;
    mount_p processes;
    BalanceState balance;
    int done;
    bool is_defer[MAX_PROCESS_ID + 1];
} proc;

enum {
    MAX_PROCESSES = 10,
};

proc myself;

size_t reader[MAX_PROCESSES][MAX_PROCESSES];
size_t writer[MAX_PROCESSES][MAX_PROCESSES];

pid_t Pids[MAX_PROCESSES];
//balance_t States[MAX_PROCESSES];

void initHistory(proc *self, balance_t initial_balance);

void goChild(proc *self, FILE *logFile,int m);

void closePipes(proc *self);

void receiveStartedInfo(proc *self);

timestamp_t get_lamport_time();

void set_lamport_time(timestamp_t new_time);

void up_time();

timestamp_t cpmLTime(timestamp_t t1, timestamp_t t2, local_id i1, local_id i2);

void updateHistory(balance_t balanse,timestamp_t time, proc *self);


#endif
