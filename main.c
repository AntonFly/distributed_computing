#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#include "banking.h"
#include "ipc.h"
#include "logging.h"
#include "process.h"
#include "messages.h"
#include "debug.h"
#include "common.h"


int main(int argc, char const *argv[]) {
    Process *self = &myself;
    self->mutual_exclusion = false;

    if (argc < 3) {
        ERROR("Please provide `-p NUM_CHILDREN'");
        return 1;
    }

    bool arg_p_met = false;

    int argi = 1;
    while (argi < argc) {
        if (strcmp(argv[argi], "--mutexl") == 0) {
            self->mutual_exclusion = true;

        } else if (strcmp(argv[argi], "-p") == 0) {
            argi++;
            if (argc <= argi) {
                ERROR("Please provide number of children after `-p'");
                return 1;
            } else {
                children = strtol(argv[argi], NULL, 10);
                processes = children + 1;

                if (children >= MAX_PROCESSES) {
                    ERROR("Too many children requested.");
                    return 1;
                }

                arg_p_met = true;
            }

        } else {
            ERROR("Expected `-p NUM_CHILDREN' or `--mutexl', found `%s'", argv[argi]);
            return 1;
        }
        argi++;
    }

    if (!arg_p_met) {
        ERROR("Option `-p NUM_CHILDREN' is not provided");
        return 1;
    }

    // Create file descriptors.
    FILE *pipes = fopen(pipes_log, "w");
    for (size_t src = 0; src < processes; src++) {
        for (size_t dest = src; dest < processes; dest++) {
            if (src != dest) {
                int pipefd_original[2];
                pipe(pipefd_original);
                fcntl(pipefd_original[0], F_SETFL, O_NONBLOCK);
                fcntl(pipefd_original[1], F_SETFL, O_NONBLOCK);
                reader[src][dest] = pipefd_original[0];
                writer[src][dest] = pipefd_original[1];
                fprintf(pipes,
                        "Pipe %2d: process %lu can READ from process %lu\n",
                        pipefd_original[0], src, dest);
                fprintf(pipes,
                        "Pipe %2d: process %lu can WRITE  to process %lu\n",
                        pipefd_original[1], src, dest);
                int pipefd_reverse[2];
                pipe(pipefd_reverse);
                fcntl(pipefd_reverse[0], F_SETFL, O_NONBLOCK);
                fcntl(pipefd_reverse[1], F_SETFL, O_NONBLOCK);
                reader[dest][src] = pipefd_reverse[0];
                writer[dest][src] = pipefd_reverse[1];
                fprintf(pipes,
                        "Pipe %2d: process %lu can READ from process %lu\n",
                        pipefd_reverse[0], dest, src);
                fprintf(pipes,
                        "Pipe %2d: process %lu can WRITE  to process %lu\n",
                        pipefd_reverse[1], dest, src);
            }
        }
    }
    fclose(pipes);

    log_init();

    self->lamport_time = 0;
    pids[PARENT_ID] = getpid();

    // Create children processes.
    for (size_t id = 1; id <= children; id++) {
        int child_pid = fork();
        if (child_pid > 0) {
            self->id = PARENT_ID;
            pids[id] = child_pid;
        }  else if (child_pid == 0) {
            self->id = id;
            break;
        } else {
            fprintf(stderr, "Fail: Forking failed");
            perror("main");
            return 1;
        }
    }


    close_other_pipes(self);


    if (self->id == PARENT_ID) {
        go_parent(self);
    } else {
        go_child(self);
    }

    log_close(self);

    while(wait(NULL) != -1);

    return 0;
}

void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    Process *self = parent_data;

    {
        self->lamport_time++;
        Message message;
        message.s_header = (MessageHeader) {
                .s_local_time = get_lamport_time(),
                .s_magic =MESSAGE_MAGIC,
                .s_type=TRANSFER,
                .s_payload_len = sizeof(TransferOrder),
        };
        TransferOrder order = {
                .s_src = src,
                .s_dst = dst,
                .s_amount = amount,
        };
        memcpy(&message.s_payload, &order, sizeof(TransferOrder));
        send(parent_data, src, &message);
    }

    {
        Message message;
        receive(parent_data, dst, &message);
        if (message.s_header.s_type == ACK) {
            up_time(self, message.s_header.s_local_time);
        } else{
            NOTICE("Expected message ACK [%d] from process %d, but got message with type [%d]",
                   ACK, dst, message.s_header.s_type);
        }
    }
}
