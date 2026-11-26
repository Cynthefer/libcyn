#define _GNU_SOURCE
#include "internal.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>

void cyn_get_gpu(cyn_ctx_t *ctx, cyn_gpu *g) {
    (void)ctx;
    memset(g, 0, sizeof *g);
    DIR *d = opendir("/sys/class/drm");
    if (!d) { cyn__unavail(&g->vendor, "/sys/class/drm missing"); return; }
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strncmp(e->d_name, "card", 4) || strchr(e->d_name, '-')) continue;
        char path[512], buf[256];
        snprintf(path, sizeof path, "/sys/class/drm/%s/device/vendor", e->d_name);
        if (cyn__read_file(path, buf, sizeof buf) == 0)
            cyn__set(&g->vendor, "%s", buf);
        snprintf(path, sizeof path, "/sys/class/drm/%s/device/device", e->d_name);
        if (cyn__read_file(path, buf, sizeof buf) == 0)
            cyn__set(&g->model, "%s", buf);
        snprintf(path, sizeof path, "/sys/class/drm/%s/device/uevent", e->d_name);
        FILE *f = fopen(path, "r");
        if (f) {
            char l[256];
            while (fgets(l, sizeof l, f))
                if (!strncmp(l, "DRIVER=", 7)) {
                    l[strcspn(l,"\n")] = 0;
                    cyn__set(&g->driver, "%s", l + 7);
                }
            fclose(f);
        }
        break;
    }
    closedir(d);    
    if (g->vendor.status == CYN_OK && g->model.status == CYN_OK) {
        const char *name = cyn_gpu_resolve_name(g->vendor.value, g->model.value);
        if (name) cyn__set(&g->model, "%s", name);
    }
}