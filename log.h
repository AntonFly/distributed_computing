/**
 * @file    log.h
 * @author  Egor Dubenetskiy, Alyona Kovalyova
 * @date    April 2019
 * @brief   Simple logging library for the assignment
 */

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

void logprintf(const char *format, ...) {
    va_list va;

    va_start(va, format);
    vprintf(format, va);
    va_end(va);

    va_start(va, format);
    vfprintf(events_log_file, format, va);
    va_end(va);
}

void log_started() {
    pid_t pid = getpid();
    pid_t parent_pid = getppid();
    logprintf(log_started_fmt, my_id, pid, parent_pid);
}

void log_received_all_started() {
    logprintf(log_received_all_started_fmt, my_id);
}

void log_done() {
    logprintf(log_done_fmt, my_id);
}

void log_received_all_done() {
    logprintf(log_received_all_done_fmt, my_id);
}

void log_close() {
    fclose(events_log_file);
}

// TODO: log_received_all_done

#endif  // __IFMO_DISTRIBUTED_CLASS_PA1_ALLEGORY__LOG__H
