#ifndef LAB1_CRITICAL_SECTION_H
#define LAB1_CRITICAL_SECTION_H

#include "process.h"

int request_cs(const Process *self_void);

int release_cs(const Process *self_void);

#endif //LAB1_CRITICAL_SECTION_H