#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "io.h"
#include "ipc.h"
#include "log.h"

void close_other_pipes();

enum {
    MAX_PROCESSES = 10,
};

local_id my_id;
size_t processes;
size_t reader[MAX_PROCESSES][MAX_PROCESSES];
size_t writer[MAX_PROCESSES][MAX_PROCESSES];

int main(int argc, char const *argv[]) {
    size_t children;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        children = strtol(argv[2], NULL, 10);
    } else {
        fprintf(stderr, "Fail: Key '-p NUMBER_OF_CHILDREN' is necessary\n");
        return 1;
    }

    if (children > 10) {
        fprintf(stderr, "Fail: Max mount of children is 9.\n");
        return 1;
    }

    processes = children + 1;

    for (size_t src = 0; src < processes; src++) {
        for (size_t dest = 0; dest < processes;
             dest++) {
            if (src != dest) {
                int pipefd[2];
                pipe(pipefd);
                reader[src][dest] = pipefd[0];
                writer[src][dest] = pipefd[1];
            }
        }
    }

    log_init();

    pid_t pids[children];
    pids[PARENT_ID] = getpid();

    for (size_t id = 1; id <= children; id++) {
        int child_pid = fork();
        if (child_pid > 0) {
            my_id = PARENT_ID;
            pids[id] = child_pid;
        } else {
            my_id = id;
            break;
        }
    }

    close_other_pipes();

    if (my_id != PARENT_ID) {
        Message msg = {.s_header ={.s_magic = MESSAGE_MAGIC,.s_type = STARTED,},};
        sprintf(msg.s_payload, log_started_fmt, my_id, getpid(), getppid());
        msg.s_header.s_payload_len = strlen(msg.s_payload);
        send_multicast(NULL, &msg);
//        log_started();
        log_msg('s');
    }

    for (size_t i = 1; i <= children; i++) {
        Message msg;
        if (i != my_id) {
            receive(NULL, i, &msg);
        }

    }
//    log_received_all_started();
    log_msg('a');
    if (my_id != PARENT_ID) {
        Message msg = {.s_header ={.s_magic = MESSAGE_MAGIC, .s_type = DONE,},};
        sprintf(msg.s_payload, log_done_fmt, my_id);
        msg.s_header.s_payload_len = strlen(msg.s_payload);
        send_multicast(NULL, &msg);
        log_msg('d');
    }

    for (size_t i = 1; i <= children; i++) {
        Message msg;
        if (i != my_id) {
            receive(NULL, i, &msg);
        }

    }
    log_msg('r');

    if (my_id == PARENT_ID) {
        for (size_t i = 1; i <= processes; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }

    log_close();
    return 0;
}

void close_other_pipes() {
    for (size_t src = 0; src < processes; src++) {
        for (size_t dest = 0; dest < processes;
             dest++) {
            if (src != my_id && dest != my_id &&
                src != dest) {
                close(writer[src][dest]);
                close(reader[src][dest]);
            }
            if (src == my_id && dest != my_id) {
                close(reader[src][dest]);
            }
            if (dest == my_id && src != my_id) {
                close(writer[src][dest]);
            }
        }
    }
}
