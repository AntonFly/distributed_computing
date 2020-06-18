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

void logPrintf(const char *format, ...) {
    va_list va;

    va_start(va, format);
    vfprintf(events_log_file, format, va);
    va_end(va);

    va_start(va, format);
    vprintf(format, va);
    va_end(va);

}

void printfLogMsg(Message *const message, const char *format, ...) {
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

void logInit() {
    events_log_file = fopen(events_log, "a");
}

void logMsg(char key, proc *self){
    pid_t pid = getpid();
    pid_t parentPid = getppid();
    switch (key){
        case 's':
            logPrintf(
                    log_started_fmt, get_physical_time, self->id,
                    pid, parentPid,
                    self->his.istoria.s_history[self->his.istoria.s_history_len - 1].s_balance);
            break;
        case 'a':
            logPrintf(log_received_all_started_fmt, get_physical_time(), self->id);
            break;
        case 'd':
            logPrintf(log_done_fmt, get_physical_time(), self->id);
            break;
        case 'r':
            logPrintf(log_received_all_done_fmt, get_physical_time(), self->id);
    }
}

void logClose(proc *self) {
    fclose(events_log_file);
}
