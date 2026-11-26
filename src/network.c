#define _GNU_SOURCE
#include "internal.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/* Helper to parse /etc/resolv.conf safely (avoids _res struct issues) */
static void get_domain(cyn_field *domain) {
    FILE *f = fopen("/etc/resolv.conf", "r");
    if (!f) {
        cyn__unavail(domain, "/etc/resolv.conf unreadable");
        return;
    }

    char line[256];
    while (fgets(line, sizeof line, f)) {
        /* Look for "domain example.com" or "search example.com" */
        if (strncmp(line, "domain ", 7) == 0) {
            char *p = line + 7;
            p[strcspn(p, "\r\n")] = 0; /* Strip newline */
            if (*p) {
                cyn__set(domain, "%s", p);
                fclose(f);
                return;
            }
        }
        if (strncmp(line, "search ", 7) == 0) {
            char *p = line + 7;
            p[strcspn(p, " \r\n")] = 0; /* Strip newline and any extra search domains */
            if (*p) {
                cyn__set(domain, "%s", p);
                fclose(f);
                return;
            }
        }
    }
    fclose(f);
    cyn__unavail(domain, "no DNS domain configured");
}

static void fetch_public_ip(cyn_ctx_t *ctx, cyn_field *out) {
    if (!ctx->opts.fetch_public_ip) {
        cyn__unavail(out, "disabled by options");
        return;
    }
    char tmp[128] = {0};
    
    /* Use the timeout from options, defaulting to 2 seconds */
    char tbuf[16];
    int secs = ctx->opts.timeout_ms > 0 ? (ctx->opts.timeout_ms / 1000) : 2;
    if (secs < 1) secs = 1;
    snprintf(tbuf, sizeof tbuf, "%d", secs);

    const char *argv[] = { "curl", "-fsS", "--max-time", tbuf,
                           "https://api.ipify.org", NULL };
                           
    if (cyn__exec_capture(argv, tmp, sizeof tmp) == 0 && *tmp)
        cyn__set(out, "%s", tmp);
    else
        cyn__unavail(out, "network unavailable");
}

void cyn_get_network(cyn_ctx_t *ctx, cyn_network *n) {
    memset(n, 0, sizeof *n);

    char host[256] = {0};
    if (gethostname(host, sizeof host - 1) == 0)
        cyn__set(&n->hostname, "%s", host);
    else
        cyn__unavail(&n->hostname, "gethostname failed");

    get_domain(&n->domain);

    struct ifaddrs *ifa, *p;
    if (getifaddrs(&ifa) == 0) {
        for (p = ifa; p; p = p->ifa_next) {
            if (!p->ifa_addr || p->ifa_addr->sa_family != AF_INET) continue;
            if (p->ifa_flags & IFF_LOOPBACK) continue;
            
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in *sin = (struct sockaddr_in *)p->ifa_addr;
            inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof ip);
            
            cyn__set(&n->interface, "%s", p->ifa_name);
            cyn__set(&n->local_ip,  "%s", ip);
            break;
        }
        freeifaddrs(ifa);
    }
    if (n->local_ip.status != CYN_OK)
        cyn__unavail(&n->local_ip, "no non-loopback IPv4");

    fetch_public_ip(ctx, &n->public_ip);
}
