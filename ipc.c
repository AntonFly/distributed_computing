#include "ipc.h"
#include <unistd.h>
#include "io.h"
#include "stdio.h"

static size_t read_exact(size_t fd, void *buf, size_t nbytes);

typedef enum {
    INVALID_PEER = 1,
    INVALID_MAGIC,
} IpcError;

int send(void *self, local_id dst, const Message *msg) {
    if (dst >= num_processes) {
        return INVALID_PEER;
    }
    if (msg->s_header.s_magic != MESSAGE_MAGIC) {
        return INVALID_MAGIC;
    }
    write(writer[my_id][dst], &msg->s_header, sizeof(MessageHeader));
    write(writer[my_id][dst], &msg->s_payload, msg->s_header.s_payload_len);
    // TODO: Loop to ensure that all bytes were sent
    return 0;
}

int send_multicast(void *self, const Message *msg) {
    for (local_id dst = 0; dst < num_processes; dst++) {
        if (dst != my_id) {
            int result = send(self, dst, msg);
            if (result > 0) {
                return result;
            }
        }
    }
    return 0;
}

int receive(void *self, local_id from, Message *msg) {
    if (from >= num_processes) {
        return INVALID_PEER;
    }

    read_exact(reader[from][my_id], &msg->s_header, sizeof(MessageHeader));
    if (msg->s_header.s_magic != MESSAGE_MAGIC) {
        return INVALID_MAGIC;
    }

    read_exact(reader[from][my_id], &msg->s_payload,
               msg->s_header.s_payload_len);
    return 0;
}

int receive_any(void *self, Message *msg) {
    // TODO: Not implemented yet
    return fprintf(stderr, "Not implemented yet");
}

/**
 * Attempts to read the exact number of bytes from file into the buffer.
 *
 * Unlike read(3P), this function will either read the exact given number of
 * bytes or exit with an error.
 */
static size_t read_exact(size_t fd, void *buf, size_t num_bytes) {
    size_t offset = 0;
    size_t remaining = num_bytes;

    while (remaining > 0) {
        int num_bytes_read = read(fd, ((char *)buf) + offset, remaining);
        if (num_bytes_read > 0) {
            remaining -= num_bytes_read;
            offset += num_bytes_read;
        } else {
            return -1;
        }
    }

    return offset;
}
