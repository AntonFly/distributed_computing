#define _XOPEN_SOURCE 600 /* needed for timespec in <time.h> */
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "ipc.h"
#include "process.h"

static size_t readExact(size_t fd, void *buf, size_t num_bytes);

typedef enum {
    INVALID_MAGIC,
} Error;

int send(void *self_void, local_id dst, const Message *msg) {
    proc *self = self_void;

    if (dst >= self->processes.procesi) {
        return 1;
    }
    if (msg->s_header.s_magic != MESSAGE_MAGIC) {
        return INVALID_MAGIC;
    }
    write(writer[self->id][dst], &msg->s_header, sizeof(MessageHeader));
    write(writer[self->id][dst], &msg->s_payload, msg->s_header.s_payload_len);
    return 0;
}

int send_multicast(void *self_void, const Message *msg) {
    proc *self = self_void;
    local_id dst = 0;
    while( dst < self->processes.procesi){
        if (dst != self->id) {
            int result = send(self, dst, msg);
            if (result > 0) {
                fprintf(stderr, "Ошибка процесс %d несмог отправить мультикаст к %d!\n", self->id, dst);
                return result;
            }
        }
        dst++;
    }
    return 0;
}

int receive(void *self_void, local_id from, Message *msg) {
    proc *self = self_void;
    if (from >= self->processes.procesi) {
        return 1;
    }

    readExact(reader[from][self->id], &msg->s_header, sizeof(MessageHeader));
    if (msg->s_header.s_magic != MESSAGE_MAGIC) {
        return INVALID_MAGIC;
    }

    readExact(reader[from][self->id], &msg->s_payload,
              msg->s_header.s_payload_len);
    return 0;
}

int receive_any(void *this, Message *msg) {
    proc *self = (proc *) this;
    int id = self->id;
    while (true) {
        if (++id == self->id) id++;
        if (id >= self->processes.procesi) {
            id -= self->processes.procesi;
        }

        size_t srcFile = reader[id][self->id];
        unsigned int flags = fcntl(srcFile, F_GETFL, 0);
        fcntl(srcFile, F_SETFL, flags | O_NONBLOCK);
        int numBytes = read(srcFile, &msg->s_header, 1);
        switch (numBytes) {
            case -1:
                nanosleep((const struct timespec[]) {{0, 1000L}}, NULL);
                continue;
            case 0: {
                nanosleep((const struct timespec[]) {{0, 1000L}}, NULL);
                continue;
            }
            default:
                break;
        }

        fcntl(srcFile, F_SETFL, flags & !O_NONBLOCK);

        read(srcFile, ((char *) &msg->s_header) + 1, sizeof(MessageHeader) - 1);
        read(srcFile, msg->s_payload, msg->s_header.s_payload_len);

        fcntl(srcFile, F_SETFL, flags | O_NONBLOCK);
        return 0;
    }
}

static size_t readExact(size_t fd, void *buf, size_t num_bytes) {
    unsigned int f = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, f & !O_NONBLOCK);

    size_t offset = 0;
    size_t remaining = num_bytes;

    while (remaining > 0) {
        int bytesRead = read(fd, ((char *)buf) + offset, remaining);
        if (bytesRead > 0) {
            remaining -= bytesRead;
            offset += bytesRead;
        } else {
            return -1;
        }
    }

    fcntl(fd, F_SETFL, f | O_NONBLOCK);
    return offset;
}






