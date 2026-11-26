#define _GNU_SOURCE
#include "internal.h"

static void dmi(cyn_field *f, const char *name) {
    char p[128];
    snprintf(p, sizeof p, "/sys/class/dmi/id/%s", name);
    CYN_TRY(f, p);
}

void cyn_get_hardware(cyn_ctx_t *ctx, cyn_hardware *h) {
    (void)ctx;
    dmi(&h->brand,   "sys_vendor");
    dmi(&h->model,   "product_name");
    dmi(&h->version, "product_version");
    dmi(&h->type,    "chassis_type");
}