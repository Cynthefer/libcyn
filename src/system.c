#define _GNU_SOURCE
#include "internal.h"
#include <sys/utsname.h>
#include <string.h>

void cyn_get_system(cyn_ctx_t *ctx, cyn_system *s) {
    (void)ctx;
    memset(s, 0, sizeof *s);
    struct utsname u;
    if (uname(&u) == 0) {
        cyn__set(&s->kernel,       "%s", u.sysname);
        cyn__set(&s->architecture, "%s", u.machine);
        cyn__set(&s->release,      "%s", u.release);
        cyn__set(&s->version,      "%s", u.version);
    } else {
        cyn__unavail(&s->kernel, "uname failed");
    }

    FILE *f = fopen("/etc/os-release", "r");
    if (!f) f = fopen("/usr/lib/os-release", "r");
    if (f) {
        char line[256];
        while (fgets(line, sizeof line, f)) {
            if (!strncmp(line, "PRETTY_NAME=", 12)) {
                char *v = line + 12;
                v[strcspn(v, "\n")] = 0;
                if (*v == '"') { v++; size_t n = strlen(v); if (n && v[n-1]=='"') v[n-1]=0; }
                cyn__set(&s->distro, "%s", v);
                break;
            }
        }
        fclose(f);
    }
    if (s->distro.status != CYN_OK)
        cyn__unavail(&s->distro, "/etc/os-release missing");
}