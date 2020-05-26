#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define ENABLE_NOTICE false
#define ENABLE_DEBUG false
#define ENABLE_ERROR true

#define __FILENAME__ (strrchr("/" __FILE__, '/') + 1)

#define __INNER_LOG(level, ...) { \
    char buf[256]; \
    int len = 0; \
    len += sprintf(buf + len, "%10s ", "[" level "]"); \
    /*len += sprintf(buf + len, "%-30s ", __FUNCTION__); \
    len += sprintf(buf + len, "(%s:%d)", __FILENAME__, __LINE__); */ \
    len += sprintf(buf + len, " [Process #%d] ", self->id); \
    len += sprintf(buf + len, __VA_ARGS__); \
    len += sprintf(buf + len, "\n"); \
    buf[len] = '\0'; \
fprintf(stderr, "%s", buf); \
}; ((void) 0)

#define DEBUG(...) if(ENABLE_DEBUG) __INNER_LOG("DEBUG", __VA_ARGS__)
#define NOTICE(...) if (ENABLE_NOTICE) __INNER_LOG("NOTICE", __VA_ARGS__)
#define ERROR(...) if (ENABLE_ERROR) __INNER_LOG("ERROR", __VA_ARGS__)

//#endif // DEBUG_H
