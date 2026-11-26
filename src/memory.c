#define _GNU_SOURCE
#include "internal.h"
#include <string.h>

void cyn_get_memory(cyn_ctx_t *ctx, cyn_memory *m) {
    (void)ctx;
    memset(m, 0, sizeof *m);
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

    /* SMBIOS/DMI memory detail is root-only — do NOT bypass. */
    cyn__unavail(&m->type,        "requires root (dmidecode)");
    cyn__unavail(&m->speed,       "requires root (dmidecode)");
    cyn__unavail(&m->form_factor, "requires root (dmidecode)");
    cyn__unavail(&m->cas_latency, "requires root (dmidecode)");
}