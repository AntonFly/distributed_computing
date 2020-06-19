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
        int m=0;
//        this->processes.deti = atoi(argv[3]);
//        this->processes.procesi = this->processes.deti + 1;
//
//        if(argc > 3 && strcmp(argv[2],"--mutexl")==0){
//            m=1;
//        }
    int a = 1;
    while (a < argc) {
        if (strcmp(argv[a], "--mutexl") == 0) {
            m = 1;

        } else if (strcmp(argv[a], "-p") == 0) {
            a++;
                this->processes.deti = strtol(argv[a], NULL, 10);
                this->processes.procesi = this->processes.deti + 1;


            }
        a++;
    }

    FILE *pippeF = fopen(pipes_log, "a");

    for (size_t src = 0; src < this->processes.procesi; src++) {
        for (size_t dest = src; dest < this->processes.procesi; dest++) {
            if (src != dest) {
                int original[2];
                pipe(original);
                fcntl(original[0], F_SETFL, O_NONBLOCK);
                fcntl(original[1], F_SETFL, O_NONBLOCK);
                reader[src][dest] = original[0];
                writer[src][dest] = original[1];
                fprintf(pippeF, "Opened for read - %d and  write - %d\n", original[0], original[1]);
                int reverse[2];
                pipe(reverse);
                fcntl(reverse[0], F_SETFL, O_NONBLOCK);
                fcntl(reverse[1], F_SETFL, O_NONBLOCK);
                reader[dest][src] = reverse[0];
                writer[dest][src] = reverse[1];
                fprintf(pippeF, "Opened for read - %d and  write - %d\n", reverse[0], reverse[1]);
            }
        }
    }

    fclose(pippeF);

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
        receiveStartedInfo(this);



        receiveDoneAll(this,logFile);

        receiveBalanceHistories(this);

        for (size_t j = 1; j <= this->processes.procesi; j++) {
        }
        while(wait(NULL)!=-1){};

        receiveDoneAll(this,logFile);


    } else {
        goChild(this,logFile,m);
    }


    fclose(logFile);
    exit(0);
    return 0;

}

void transfer(void *parent_data, local_id src, local_id dst, balance_t mount) {

    up_time();
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
                .s_local_time = get_lamport_time(),
        };
        memcpy(&mesg.s_payload, &ord, sizeof(TransferOrder));
        send(parent_data, src, &mesg);
    }

    {
        receive(parent_data, dst, &mesg);
        set_lamport_time(cpmLTime(get_lamport_time(),mesg.s_header.s_local_time,0,0));
        up_time();
    }
}
