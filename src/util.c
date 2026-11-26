#define _GNU_SOURCE
#include "internal.h"
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static __thread char g_err[256];

void cyn__set_error(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vsnprintf(g_err, sizeof g_err, fmt, ap);
    va_end(ap);
}
const char *cyn_last_error(void) { return g_err; }

const char *cyn_status_str(cyn_status s) {
    switch (s) {
        case CYN_OK:          return "ok";
        case CYN_UNAVAILABLE: return "unavailable";
        case CYN_ERROR:       return "error";
    }
    return "?";
}

void cyn__set(cyn_field *f, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vsnprintf(f->value, sizeof f->value, fmt, ap);
    va_end(ap);
    f->status = CYN_OK;
}

void cyn__unavail(cyn_field *f, const char *why) {
    snprintf(f->value, sizeof f->value, "%s", why ? why : "unavailable");
    f->status = CYN_UNAVAILABLE;
}

static void rtrim(char *s) {
    size_t n = strlen(s);
    while (n && (s[n-1]=='\n'||s[n-1]=='\r'||s[n-1]==' '||s[n-1]=='\t'))
        s[--n] = 0;
}

int cyn__read_file(const char *path, char *buf, size_t len) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    if (!fgets(buf, (int)len, f)) { fclose(f); return -1; }
    rtrim(buf);
    fclose(f);
    return 0;
}

int cyn__exec_capture(const char *const argv[], char *buf, size_t len) {
    int pfd[2];
    if (pipe(pfd) != 0) return -1;
    pid_t pid = fork();
    if (pid < 0) { close(pfd[0]); close(pfd[1]); return -1; }
    if (pid == 0) {
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[0]); close(pfd[1]);
        execvp(argv[0], (char *const *)argv);
        _exit(127);
    }
    close(pfd[1]);
    ssize_t n = read(pfd[0], buf, len - 1);
    close(pfd[0]);
    int st; waitpid(pid, &st, 0);
    if (n < 0) return -1;
    buf[n] = 0; rtrim(buf);
    return WIFEXITED(st) && WEXITSTATUS(st) == 0 ? 0 : -1;
}