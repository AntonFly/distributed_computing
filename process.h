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
#include "banking.h"

#include "ipc.h"

typedef struct {
    BalanceHistory history;
    local_id id;
    AllHistory all_history;
} Process;

enum {
    MAX_PROCESSES = 10,
};

Process myself;
size_t processes;
size_t children;

size_t reader[MAX_PROCESSES][MAX_PROCESSES];
size_t writer[MAX_PROCESSES][MAX_PROCESSES];

pid_t pids[MAX_PROCESSES];
balance_t states[MAX_PROCESSES];

void initialize_history(Process *self, balance_t initial_balance);

void go_parent(Process *self);

void go_child(Process *self, balance_t initial_balance);

void close_other_pipes(Process *self);

#endif
