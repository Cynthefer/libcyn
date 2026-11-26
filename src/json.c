#include "internal.h"
#include <stdlib.h>
#include <string.h>

/* Escape a string for JSON. Caller frees. */
static char *jesc(const char *s) {
    size_t n = strlen(s);
    char *out = malloc(n * 6 + 1);
    if (!out) return NULL;
    char *p = out;
    for (const unsigned char *c = (const unsigned char *)s; *c; c++) {
        switch (*c) {
            case '"':  *p++='\\'; *p++='"';  break;
            case '\\': *p++='\\'; *p++='\\'; break;
            case '\n': *p++='\\'; *p++='n';  break;
            case '\r': *p++='\\'; *p++='r';  break;
            case '\t': *p++='\\'; *p++='t';  break;
            default:
                if (*c < 0x20) { p += sprintf(p, "\\u%04x", *c); }
                else *p++ = (char)*c;
        }
    }
    *p = 0;
    return out;
}

/* Write field as {"value": "...", "status": "ok"} */
static void jfield(FILE *f, const char *key, const cyn_field *v, int last) {
    char *e = jesc(v->value);
    fprintf(f, "    \"%s\": {\"value\": \"%s\", \"status\": \"%s\"}%s\n",
            key, e ? e : "", cyn_status_str(v->status), last ? "" : ",");
    free(e);
}

char *cyn_to_json(const cyn_info *i) {
    if (!i) return NULL;
    char *buf = NULL;
    size_t len = 0;
    FILE *f = open_memstream(&buf, &len);
    if (!f) return NULL;

    fprintf(f, "{\n");
    fprintf(f, "  \"user\": {\n");
    jfield(f, "username", &i->user.username, 0);
    jfield(f, "fullname", &i->user.fullname, 0);
    jfield(f, "uid",      &i->user.uid,      0);
    jfield(f, "gid",      &i->user.gid,      0);
    jfield(f, "home",     &i->user.home,     0);
    jfield(f, "shell",    &i->user.shell,    1);
    fprintf(f, "  },\n");

    fprintf(f, "  \"system\": {\n");
    jfield(f, "kernel",       &i->system.kernel,       0);
    jfield(f, "architecture", &i->system.architecture, 0);
    jfield(f, "release",      &i->system.release,      0);
    jfield(f, "version",      &i->system.version,      0);
    jfield(f, "distro",       &i->system.distro,       1);
    fprintf(f, "  },\n");

    fprintf(f, "  \"network\": {\n");
    jfield(f, "hostname",   &i->network.hostname,   0);
    jfield(f, "domain",     &i->network.domain,     0);
    jfield(f, "interface",  &i->network.interface,  0);
    jfield(f, "local_ip",   &i->network.local_ip,   0);
    jfield(f, "public_ip",  &i->network.public_ip,  1);
    fprintf(f, "  },\n");

    fprintf(f, "  \"memory\": {\n");
    jfield(f, "size",        &i->memory.size,        0);
    jfield(f, "type",        &i->memory.type,        0);
    jfield(f, "speed",       &i->memory.speed,       0);
    jfield(f, "form_factor", &i->memory.form_factor, 0);
    jfield(f, "cas_latency", &i->memory.cas_latency, 1);
    fprintf(f, "  },\n");

    fprintf(f, "  \"storage\": {\n");
    jfield(f, "home_dir", &i->storage.home_dir, 0);
    jfield(f, "total",    &i->storage.total,    0);
    jfield(f, "used",     &i->storage.used,     0);
    jfield(f, "free",     &i->storage.free,     1);
    fprintf(f, "  },\n");

    fprintf(f, "  \"cpu\": {\n");
    jfield(f, "vendor", &i->cpu.vendor, 0);
    jfield(f, "model",  &i->cpu.model,  0);
    jfield(f, "cores",  &i->cpu.cores,  1);
    fprintf(f, "  },\n");

    fprintf(f, "  \"gpu\": {\n");
    jfield(f, "vendor", &i->gpu.vendor, 0);
    jfield(f, "model",  &i->gpu.model,  0);
    jfield(f, "driver", &i->gpu.driver, 1);
    fprintf(f, "  },\n");

    fprintf(f, "  \"hardware\": {\n");
    jfield(f, "brand",   &i->hardware.brand,   0);
    jfield(f, "model",   &i->hardware.model,   0);
    jfield(f, "type",    &i->hardware.type,    0);
    jfield(f, "version", &i->hardware.version, 1);
    fprintf(f, "  }\n");

    fprintf(f, "}\n");
    fclose(f);
    return buf;
}

void cyn_json_free(char *json) { free(json); }