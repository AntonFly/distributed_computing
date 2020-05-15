#include "process.h"
#include "pa2345.h"
#include "ipc.h"
#include "common.h"
#include "unistd.h"
#include "sys/types.h"
#include "stdarg.h"
#include "signal.h"
#include "stdio.h"
#include "logging.h"

static FILE *events_log_file;

void log_printf(const char *format, ...) {
    va_list va;

    va_start(va, format);
    vprintf(format, va);
    va_end(va);

    va_start(va, format);
    vfprintf(events_log_file, format, va);
    va_end(va);
}

void printf_log_msg(Message *const message, const char *format, ...) {
    va_list va;

    va_start(va, format);
    vprintf(format, va);
    va_end(va);

    va_start(va, format);
    vfprintf(events_log_file, format, va);
    va_end(va);

    va_start(va, format);
    size_t payload_length = sprintf(message->s_payload, format, va);
    message->s_header.s_payload_len = payload_length;
    va_end(va);
}

void log_init() {
    events_log_file = fopen(events_log, "a");
}

void log_msg(char key,Process *self){
    pid_t pid = getpid();
    pid_t parent_pid = getppid();
    switch (key){
        case 's':
            log_printf(
                    log_started_fmt, get_physical_time, self->id,
                    pid, parent_pid,
                    self->history.s_history[self->history.s_history_len - 1].s_balance);
            break;
        case 'a':
            log_printf(log_received_all_started_fmt, get_physical_time(), self->id);
            break;
        case 'd':
            log_printf(log_done_fmt, get_physical_time(), self->id);
            break;
        case 'r':
            log_printf(log_received_all_done_fmt, get_physical_time(), self->id);
    }
}

void log_close(Process *self) {
    fclose(events_log_file);
}
