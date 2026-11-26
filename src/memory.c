#define _GNU_SOURCE
#include "internal.h"
#include <string.h>
#include <unistd.h>

void cyn_get_memory(cyn_ctx_t *ctx, cyn_memory *m) {
    (void)ctx;
    memset(m, 0, sizeof *m);
    
    /* 1. Get RAM Size (No root required) */
    FILE *f = fopen("/proc/meminfo", "r");
    if (f) {
        char key[64]; unsigned long val; char unit[16];
        while (fscanf(f, "%63s %lu %15s", key, &val, unit) == 3) {
            if (!strcmp(key, "MemTotal:")) {
                cyn__set(&m->size, "%.1f GiB", val / 1024.0 / 1024.0);
                break;
            }
        }
        fclose(f);
    }
    if (m->size.status != CYN_OK)
        cyn__unavail(&m->size, "/proc/meminfo unreadable");

    /* 2. Get RAM Details (Requires root) */
    if (getuid() != 0) {
        cyn__unavail(&m->type,        "root needed");
        cyn__unavail(&m->speed,       "root needed");
        cyn__unavail(&m->form_factor, "root needed");
        cyn__unavail(&m->cas_latency, "root needed");
        return;
    }

    /* We are root, try to use dmidecode */
    char buf[4096] = {0};
    const char *argv[] = { "dmidecode", "-t", "memory", NULL };
    
    if (cyn__exec_capture(argv, buf, sizeof buf) == 0) {
        char *line = strtok(buf, "\n");
        while (line) {
            /* Skip leading whitespace */
            char *p = line;
            while (*p == ' ' || *p == '\t') p++;

            if (strncmp(p, "Type:", 5) == 0 && m->type.status != CYN_OK) {
                char *val = p + 5;
                while (*val == ' ') val++;
                if (strcmp(val, "Unknown") != 0 && strcmp(val, "Other") != 0) {
                    cyn__set(&m->type, "%s", val);
                }
            } 
            else if (strncmp(p, "Speed:", 6) == 0 && m->speed.status != CYN_OK) {
                char *val = p + 6;
                while (*val == ' ') val++;
                if (strcmp(val, "Unknown") != 0) {
                    cyn__set(&m->speed, "%s", val);
                }
            }
            else if (strncmp(p, "Form Factor:", 12) == 0 && m->form_factor.status != CYN_OK) {
                char *val = p + 12;
                while (*val == ' ') val++;
                if (strcmp(val, "Unknown") != 0) {
                    cyn__set(&m->form_factor, "%s", val);
                }
            }
            else if (strncmp(p, "CAS Latency:", 12) == 0 && m->cas_latency.status != CYN_OK) {
                char *val = p + 12;
                while (*val == ' ') val++;
                if (strcmp(val, "Unknown") != 0) {
                    cyn__set(&m->cas_latency, "%s", val);
                }
            }

            line = strtok(NULL, "\n");
        }
    }

    /* Fallbacks if dmidecode wasn't installed or didn't output values */
    if (m->type.status != CYN_OK)        cyn__unavail(&m->type, "dmidecode unavailable");
    if (m->speed.status != CYN_OK)       cyn__unavail(&m->speed, "dmidecode unavailable");
    if (m->form_factor.status != CYN_OK) cyn__unavail(&m->form_factor, "dmidecode unavailable");
    if (m->cas_latency.status != CYN_OK) cyn__unavail(&m->cas_latency, "dmidecode unavailable");
}
