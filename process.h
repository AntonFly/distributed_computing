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
#include "priority.h"

#define CONVERT_SELF_TO(ref_type, var_name) ref_type* self = ((ref_type *) var_name)

enum {
    MAX_PROCESSES = 10,
};

typedef struct {
    BalanceHistory history;
    local_id id;
    AllHistory all_history;
    timestamp_t lamport_time;
    bool mutual_exclusion;
    int done_received;
    PriorityQueue request_queue;
    bool is_defer[MAX_PROCESS_ID + 1];
} Process;


Process myself;
size_t processes;
size_t children;

size_t reader[MAX_PROCESSES][MAX_PROCESSES];
size_t writer[MAX_PROCESSES][MAX_PROCESSES];

pid_t pids[MAX_PROCESSES];
balance_t states[MAX_PROCESSES];

void init_history(Process *self, balance_t initial_balance);

void go_parent(Process *self);

void go_child(Process *self);

void close_other_pipes(Process *self);

timestamp_t get_lamport_time();

timestamp_t lift_and_get_local_time(Process *self, timestamp_t their_time);

timestamp_t increment_and_get_local_time(Process *self);

void up_time(Process *self, timestamp_t their_time);

const char *msg_type_to_string(MessageType type);

int get_right_fork_index(Process* self);

int get_left_fork_index(Process* self);

int have_all_forks(Process* self);

void send_cs_request(Process* self, local_id to);

#endif
