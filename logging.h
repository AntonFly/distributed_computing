#ifndef LAB1_LOGGING_H
#define LAB1_LOGGING_H

#include "process.h"

void log_printf(const char *format, ...);

void printf_log_msg(Message *message, const char *format, ...);

void log_init();

void log_msg(char key,Process *self);

void log_close(Process *self);

#endif
