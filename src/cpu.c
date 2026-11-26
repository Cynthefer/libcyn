#define _GNU_SOURCE
#include "internal.h"
#include <string.h>

void cyn_get_cpu(cyn_ctx_t *ctx, cyn_cpu *c) {
    (void)ctx;
    memset(c, 0, sizeof *c);
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (!f) { cyn__unavail(&c->vendor, "/proc/cpuinfo unreadable"); return; }
    char line[512]; int cores = 0;
    while (fgets(line, sizeof line, f)) {
        if (!strncmp(line, "processor", 9)) cores++;
        if (!strncmp(line, "vendor_id", 9) && c->vendor.status != CYN_OK) {
            char *p = strchr(line, ':'); if (p) { p++; while (*p==' ') p++;
                p[strcspn(p,"\n")] = 0; cyn__set(&c->vendor, "%s", p); }
        }
        if (!strncmp(line, "model name", 10) && c->model.status != CYN_OK) {
            char *p = strchr(line, ':'); if (p) { p++; while (*p==' ') p++;
                p[strcspn(p,"\n")] = 0; cyn__set(&c->model, "%s", p); }
        }
    }
    fclose(f);
    if (cores) cyn__set(&c->cores, "%d", cores);
}