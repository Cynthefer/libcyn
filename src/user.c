#define _GNU_SOURCE
#include "internal.h"
#include <pwd.h>
#include <unistd.h>
#include <string.h>

void cyn_get_user(cyn_ctx_t *ctx, cyn_user *u) {
    (void)ctx;
    memset(u, 0, sizeof *u);
    struct passwd *pw = getpwuid(getuid());
    if (!pw) {
        cyn__unavail(&u->username, "getpwuid failed");
        return;
    }
    cyn__set(&u->username, "%s", pw->pw_name);
    cyn__set(&u->uid, "%u", (unsigned)pw->pw_uid);
    cyn__set(&u->gid, "%u", (unsigned)pw->pw_gid);
    cyn__set(&u->home, "%s", pw->pw_dir  ? pw->pw_dir  : "?");
    cyn__set(&u->shell, "%s", pw->pw_shell? pw->pw_shell: "?");

    if (pw->pw_gecos && *pw->pw_gecos) {
        char gecos[256];
        snprintf(gecos, sizeof gecos, "%s", pw->pw_gecos);
        char *c = strchr(gecos, ','); if (c) *c = 0;
        cyn__set(&u->fullname, "%s", gecos);
    } else {
        cyn__unavail(&u->fullname, "not set in passwd");
    }
}