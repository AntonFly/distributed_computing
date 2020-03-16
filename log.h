#ifndef __IFMO_DISTRIBUTED_CLASS_PA1_ALLEGORY__LOG__H
#define __IFMO_DISTRIBUTED_CLASS_PA1_ALLEGORY__LOG__H

#include "stdio.h"

#include "signal.h"
#include "stdarg.h"
#include "sys/types.h"
#include "unistd.h"

#include "common.h"
#include "ipc.h"
#include "pa1.h"

static FILE *events_log_file;
local_id my_id;

void log_init() {
    events_log_file = fopen(events_log, "w");
}

void log_printf(const char *format, ...) {
    va_list va;

    va_start(va, format);
    vprintf(format, va);
    va_end(va);

    va_start(va, format);
    vfprintf(events_log_file, format, va);
    va_end(va);
}

//void log_started() {
//    pid_t pid = getpid();
//    pid_t parent_pid = getppid();
//    log_printf(log_started_fmt, my_id, pid, parent_pid);
//}
//
//void log_received_all_started() {
//    log_printf(log_received_all_started_fmt, my_id);
//}
//
//void log_done() {
//    log_printf(log_done_fmt, my_id);
//}

void log_msg(char key){
    pid_t pid = getpid();
    pid_t parent_pid = getppid();
    switch (key){
        case 's':
            log_printf(log_started_fmt, my_id, pid, parent_pid);
            break;
        case 'a':
            log_printf(log_received_all_started_fmt, my_id);
            break;
        case 'd':
            log_printf(log_done_fmt, my_id);
            break;
        case 'r':
            log_printf(log_received_all_done_fmt, my_id);
    }

}


void log_close() {
    fclose(events_log_file);
}

// TODO: log_received_all_done

#endif  // __IFMO_DISTRIBUTED_CLASS_PA1_ALLEGORY__LOG__H
