#ifndef CYN_INTERNAL_H
#define CYN_INTERNAL_H
#include "cyn.h"
#include <stdio.h>

struct cyn_ctx {
    cyn_options opts;
    /* cache: NULL until first access */
    cyn_info   *cache;
    int         cache_valid;
};

void cyn__set(cyn_field *f, const char *fmt, ...);
void cyn__unavail(cyn_field *f, const char *why);
int  cyn__read_file(const char *path, char *buf, size_t len);
int  cyn__exec_capture(const char *const argv[], char *buf, size_t len);
void cyn__set_error(const char *fmt, ...);

#define CYN_TRY(field, path) do { \
    char _b[256]; \
    if (cyn__read_file((path), _b, sizeof _b) == 0) \
        cyn__set((field), "%s", _b); \
    else \
        cyn__unavail((field), "unreadable: " #path); \
} while (0)

#endif