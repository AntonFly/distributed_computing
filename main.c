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


int main(int argc, char const *argv[]) {
    Process *self = &myself;

    if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
        self->children = strtol(argv[2], NULL, 10);
        self->processes = self->children + 1;

        if (self->children > 9) {
            fprintf(stderr, "Fail: Max mount of children is 9.\n");
            return 1;
        }

        if (argc != 3 + self->children) {
            fprintf(stderr, "Fail: Expected %ld state after `%s %s'\n",
                    self->children, argv[1], argv[2]);
            return 1;
        }

        for (size_t i = 1; i <= self->children; i++) {
            states[i] = strtol(argv[2 + i], NULL, 10);
        }
    } else {
        fprintf(stderr, "Fail: Key '-p NUMBER_OF_CHILDREN' is necessary\n");
        return 1;
    }

    // Create file descriptors.
    for (size_t src = 0; src < self->processes; src++) {
        for (size_t dest = src; dest < self->processes; dest++) {
            if (src != dest) {
                int pipefd_original[2];
                pipe(pipefd_original);
                fcntl(pipefd_original[0], F_SETFL, O_NONBLOCK);
                fcntl(pipefd_original[1], F_SETFL, O_NONBLOCK);
                reader[src][dest] = pipefd_original[0];
                writer[src][dest] = pipefd_original[1];
                int pipefd_reverse[2];
                pipe(pipefd_reverse);
                fcntl(pipefd_reverse[0], F_SETFL, O_NONBLOCK);
                fcntl(pipefd_reverse[1], F_SETFL, O_NONBLOCK);
                reader[dest][src] = pipefd_reverse[0];
                writer[dest][src] = pipefd_reverse[1];
            }
        }
    }

    log_init();

//    pid_t pids[children];
    pids[PARENT_ID] = getpid();

    // Create children processes.
    for (size_t id = 1; id <= self->children; id++) {
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
        go_child(self, states[self->id]);
    }

    log_close(self);
    return 0;
}

void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    Process *self = parent_data;

    DEBUG("Process %d: Transferring $%d from #%d to #%d\n", self->id, amount, src, dst);
    Message message;
    {
        message.s_header = (MessageHeader) {
                .s_local_time = get_physical_time(),
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

    DEBUG("Process %d: Waiting for ACK from %d\n", self->id, dst);
    {
        receive(parent_data, dst, &message);
        if (message.s_header.s_type != ACK) {
            fprintf(stderr,
                    "\u26A0 "  // Unicode WARNING SIGN
                    "Warning: Process %d expected message ACK [%d] "
                    "from process %d, "
                    "got message with type [%d]\n",
                    self->id, ACK, dst, message.s_header.s_type);
        }
    }
}
