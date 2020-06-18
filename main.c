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
#include "process.h"
#include "messages.h"
#include "common.h"


int main(int argc, char const *argv[]) {
    proc *this = &myself;

        this->processes.deti = atoi(argv[2]);
        this->processes.procesi = this->processes.deti + 1;

        size_t i = 1;
        while (i <= this->processes.deti){
            States[i] = atoi(argv[2 + i]);
            i++;
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

    FILE *logFile = fopen(events_log, "a");

    size_t id = 1;
    while (id <= this->processes.deti){
        int childPid = fork();
        if (childPid > 0) {
            this->id = PARENT_ID;
            Pids[id] = childPid;
        }  else if (childPid == 0) {
            this->id = id;
            break;
        }
        id++;
    }

    closePipes(this);


    if (this->id == PARENT_ID) {
        receiveStartedAll(this, logFile);

        bank_robbery(this, this->processes.procesi - 1);

        receiveStartedInfo(this);


        stopAll(this);

        receiveDoneAll(this,logFile);

        receiveBalanceHistories(this);

        for (size_t j = 1; j <= this->processes.procesi; j++) {
        }

        print_history(&this->his.vsiaIstoria);

        while(wait(NULL)!=-1){};
    } else {
        goChild(this, States[this->id], logFile);
    }


    fclose(logFile);

    return 0;
}

void transfer(void *parent_data, local_id src, local_id dst, balance_t mount) {

    Message mesg;
    {
        TransferOrder ord = {
                .s_amount = mount,
                .s_dst = dst,
                .s_src = src,
        };
        mesg.s_header = (MessageHeader) {
                .s_payload_len = sizeof(TransferOrder),
                .s_type=TRANSFER,
                .s_magic =MESSAGE_MAGIC,
                .s_local_time = get_physical_time(),
        };
        memcpy(&mesg.s_payload, &ord, sizeof(TransferOrder));
        send(parent_data, src, &mesg);
    }

    {
        receive(parent_data, dst, &mesg);
    }
}
