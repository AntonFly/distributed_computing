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
    Process *this = &myself;
    printf("main 22");

    if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
        this->processes.deti = strtol(argv[2], NULL, 10);
        this->processes.procesi = this->processes.deti + 1;

        for (size_t i = 1; i <= this->processes.deti; i++) {
            States[i] = strtol(argv[2 + i], NULL, 10);
        }
    }

    for (size_t src = 0; src < this->processes.procesi; src++) {
        for (size_t dest = src; dest < this->processes.procesi; dest++) {
            if (src != dest) {
                int original[2];
                pipe(original);
                fcntl(original[0], F_SETFL, O_NONBLOCK);
                fcntl(original[1], F_SETFL, O_NONBLOCK);
                reader[src][dest] = original[0];
                writer[src][dest] = original[1];
                int reverse[2];
                pipe(reverse);
                fcntl(reverse[0], F_SETFL, O_NONBLOCK);
                fcntl(reverse[1], F_SETFL, O_NONBLOCK);
                reader[dest][src] = reverse[0];
                writer[dest][src] = reverse[1];
            }
        }
    }


    Pids[PARENT_ID] = getpid();

    logInit();

    for (size_t id = 1; id <= this->processes.deti; id++) {
        int childPid = fork();
        if (childPid > 0) {
            this->id = PARENT_ID;
            Pids[id] = childPid;
        }  else if (childPid == 0) {
            this->id = id;
            break;
        }
    }


    closeOtherPipes(this);

    printf("after fork main.c 70");
    if (this->id == PARENT_ID) {
        goParent(this);
    } else {
        goChild(this, States[this->id]);
    }

    logClose(this);
    return 0;
}

void transfer(void *parent_data, local_id src, local_id dst, balance_t mount) {
    Process *self = parent_data;

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
                .s_amount = mount,
        };
        memcpy(&message.s_payload, &order, sizeof(TransferOrder));
        send(parent_data, src, &message);
    }

    {
        receive(parent_data, dst, &message);
        if (message.s_header.s_type != ACK) {
            fprintf(stderr,
                    "\u26A0 "  // Unicode WARNING SIGN
                    "Ошибка процесс %d ожидал сообщение ACK [%d] "
                    "от процесса %d, "
                    "а получил сообщение типа [%d]\n",
                    self->id, ACK, dst, message.s_header.s_type);
        }
    }
}
