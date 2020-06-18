#ifndef LAB1_LOGGING_H
#define LAB1_LOGGING_H

#include "process.h"

void logPrintf(const char *format, ...);

void printfLogMsg(Message *message, const char *format, ...);

void logInit();

void logMsg(char key, proc *self);

void logClose(proc *self);

#endif
