#define _GNU_SOURCE
#include "internal.h"
#include <sys/statvfs.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>

static void fmt(char *buf, size_t len, unsigned long long bytes) {
    const char *u[] = {"B","KiB","MiB","GiB","TiB"};
    double v = (double)bytes; int i = 0;
    while (v >= 1024.0 && i < 4) { v /= 1024.0; i++; }
    snprintf(buf, len, "%.1f %s", v, u[i]);
}

void cyn_get_storage(cyn_ctx_t *ctx, cyn_storage *s) {
    (void)ctx;
    memset(s, 0, sizeof *s);
    struct passwd *pw = getpwuid(getuid());
    const char *home = (pw && pw->pw_dir) ? pw->pw_dir : ".";
    cyn__set(&s->home_dir, "%s", home);

    struct statvfs sv;
    if (statvfs(home, &sv) != 0) {
        cyn__unavail(&s->total, "statvfs failed");
        cyn__unavail(&s->used,  "statvfs failed");
        cyn__unavail(&s->free,  "statvfs failed");
        return;
    }
    unsigned long long bs = sv.f_frsize;
    unsigned long long total = (unsigned long long)sv.f_blocks * bs;
    unsigned long long freeb = (unsigned long long)sv.f_bavail * bs;
    unsigned long long used  = total - freeb;
    char b[64];
    fmt(b, sizeof b, total); cyn__set(&s->total, "%s", b);
    fmt(b, sizeof b, used);  cyn__set(&s->used,  "%s", b);
    fmt(b, sizeof b, freeb); cyn__set(&s->free,  "%s", b);
}