#ifndef LAB1_DEBUG_H
#define LAB1_DEBUG_H

#include <stdarg.h>
#include <stdio.h>

//#define ENABLE_DEBUGGING

#ifdef ENABLE_DEBUGGING
#define DEBUG(...) fprintf(stdout, "/DEBUG/ " __VA_ARGS__)
#else
#define DEBUG(...) if (0) fprintf(stderr, "/DEBUG/" __VA_ARGS__)
#endif // ENABLE_DEBUGGING

/*
inline void debug(const char *format, ...) {
#ifdef DEBUG
    va_list va;

    va_start(va, format);
    vprintf(format, va);
    va_end(va);
#endif
}
 */

#endif //LAB1_DEBUG_H
