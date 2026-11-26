#include "internal.h"
#include <stdlib.h>
#include <string.h>

cyn_ctx_t *cyn_create(const cyn_options *opts) {
    cyn_ctx_t *c = calloc(1, sizeof *c);
    if (!c) return NULL;
    if (opts) c->opts = *opts;
    if (c->opts.timeout_ms <= 0) c->opts.timeout_ms = 2000;
    return c;
}

void cyn_destroy(cyn_ctx_t *c) {
    if (!c) return;
    free(c->cache);
    free(c);
}

cyn_status cyn_collect(cyn_ctx_t *c, cyn_info *out) {
    if (!c || !out) return CYN_ERROR;
    memset(out, 0, sizeof *out);
    cyn_get_user(c, &out->user);
    cyn_get_system(c, &out->system);
    cyn_get_network(c, &out->network);
    cyn_get_memory(c, &out->memory);
    cyn_get_storage(c, &out->storage);
    cyn_get_cpu(c, &out->cpu);
    cyn_get_gpu(c, &out->gpu);
    cyn_get_hardware(c, &out->hardware);
    return CYN_OK;
}

/* ---------- Convenience single-field accessors ---------- */

static const char *maybe(const cyn_field *f) {
    return f->status == CYN_OK ? f->value : NULL;
}

static cyn_info *get_cache(cyn_ctx_t *c) {
    if (!c) return NULL;
    if (!c->cache) {
        c->cache = calloc(1, sizeof *c->cache);
        if (!c->cache) return NULL;
        cyn_collect(c, c->cache);
        c->cache_valid = 1;
    }
    return c->cache;
}

#define ACCESSOR(name, expr) \
    const char *name(cyn_ctx_t *c) { \
        cyn_info *i = get_cache(c); \
        return i ? maybe(expr) : NULL; \
    }

ACCESSOR(cyn_user_username,   &i->user.username)
ACCESSOR(cyn_user_fullname,   &i->user.fullname)
ACCESSOR(cyn_system_kernel,   &i->system.kernel)
ACCESSOR(cyn_system_distro,   &i->system.distro)
ACCESSOR(cyn_network_hostname,&i->network.hostname)
ACCESSOR(cyn_network_public_ip,&i->network.public_ip)
ACCESSOR(cyn_cpu_model,       &i->cpu.model)
ACCESSOR(cyn_hardware_model,  &i->hardware.model)