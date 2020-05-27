#ifndef LAB1_CRITICAL_SECTION_H
#define LAB1_CRITICAL_SECTION_H

#include "process.h"

int request_critical(Process *self);

int release_critical(Process *self);

int request_cs(const void *self);

int release_cs(const void *self);

#endif //LAB1_CRITICAL_SECTION_H
