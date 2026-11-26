#define _GNU_SOURCE
#include "internal.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <resolv.h>

static void fetch_public_ip(cyn_ctx_t *ctx, cyn_field *out) {
    if (!ctx->opts.fetch_public_ip) {
        cyn__unavail(out, "disabled by options");
        return;
    }
    char tmp[128] = {0};
    const char *argv[] = { "curl", "-fsS", "--max-time", "2",
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

    res_init();
    if (_res.defdname && *_res.defdname)
        cyn__set(&n->domain, "%s", _res.defdname);
    else
        cyn__unavail(&n->domain, "no DNS domain");

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