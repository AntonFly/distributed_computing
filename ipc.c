#define _XOPEN_SOURCE 600 /* needed for timespec in <time.h> */
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "ipc.h"
#include "process.h"
static size_t try_read(size_t fd, void *buf, size_t num_bytes);

static size_t read_exact(size_t fd, void *buf, size_t num_bytes);

inline void not_block(size_t fd);

inline void block(size_t fd);

typedef enum {
    INVALID_PEER = 1,
    INVALID_MAGIC,
} IpcError;

int send(void *self_void, local_id dst, Message const *msg) {
    Process *self = self_void;

    if (dst >= processes) {
        return INVALID_PEER;
    }
    if (msg->s_header.s_magic != MESSAGE_MAGIC) {
        return INVALID_MAGIC;
    }

    self->lamport_time++;

    write(writer[self->id][dst], &msg->s_header, sizeof(MessageHeader));
    write(writer[self->id][dst], &msg->s_payload, msg->s_header.s_payload_len);
    return 0;
}

int send_multicast(void *self_void, const Message *msg) {
    Process *self = self_void;
    for (local_id dst = 0; dst < processes; dst++) {
        if (dst != self->id) {
            int result = send(self, dst, msg);
            if (result > 0) {
                fprintf(stderr, "Fail: Process %d fail to send_multicast when send to %d!\n", self->id, dst);
                return result;
            }
        }
    }
    return 0;
}

int receive(void *self_void, local_id from, Message *msg) {
    Process *self = self_void;
    if (from >= processes) {
        return INVALID_PEER;
    }

    read_exact(reader[from][self->id], &msg->s_header, sizeof(MessageHeader));
    if (msg->s_header.s_magic != MESSAGE_MAGIC) {
        return INVALID_MAGIC;
    }

    read_exact(reader[from][self->id], &msg->s_payload,
               msg->s_header.s_payload_len);
    return 0;
}

int receive_any(void *self_void, Message *msg) {
    Process *self = (Process *) self_void;
    int src = self->id;
    while (true) {
        if (++src == self->id) src++;
        if (src >= processes) {
            src -= processes;
        }

        size_t src_file = reader[src][self->id];
        int num_bytes_read = try_read(src_file, &msg->s_header, sizeof(MessageHeader));
        if (num_bytes_read == -1 && errno == EAGAIN) {
            // Would block, go to next
            nanosleep((const struct timespec[]) {{0, 1000L}}, NULL);
            continue;
        } else if (num_bytes_read == 0) {
            // EOF reached
            nanosleep((const struct timespec[]) {{0, 1000L}}, NULL);
            continue;
        } else {
            // Header is read fully, continue reading
            read_exact(src_file, msg->s_payload, msg->s_header.s_payload_len);
            return 0;
        }
    }
}

inline void not_block(size_t fd) {
    unsigned int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

inline void block(size_t fd) {
    unsigned int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & !O_NONBLOCK);
}

static size_t read_exact(size_t fd, void *buf, size_t num_bytes) {
    size_t offset = 0;
    size_t remaining = num_bytes;

    while (remaining > 0) {
        int num_bytes_read = read(fd, ((char *)buf) + offset, remaining);
        if (num_bytes_read > 0) {
            remaining -= num_bytes_read;
            offset += num_bytes_read;
        } else if (num_bytes_read == -1 && errno == EAGAIN) {
            continue;
        } else {
            return -1;
        }
    }

    return offset;
}

size_t try_read(size_t fd, void *buf, size_t num_bytes) {
    size_t offset = 0;
    size_t left = num_bytes;

    {
        int bytes = read(fd, ((char *) buf) + offset, left);
        if (bytes > 0) {
            left -= bytes;
            offset += bytes;
        } else {
            return bytes;
        }
    }

    while (left > 0) {
        int bytes = read(fd, ((char *) buf) + offset, left);
        if (bytes > 0) {
            left -= bytes;
            offset += bytes;
        } else if (bytes == -1 && errno == EAGAIN) {
            continue;
        } else {
            return bytes;
        }
    }

    return offset;
}
