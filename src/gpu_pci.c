#define _GNU_SOURCE
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char g_pci_path[512] = {0};

void cyn_set_pci_ids_path(const char *path) {
    snprintf(g_pci_path, sizeof g_pci_path, "%s", path ? path : "");
}

/* Normalize "0x10de" / "10DE" / "10de" -> "10de" */
static void norm_id(const char *in, char out[5]) {
    if (!in) { out[0] = 0; return; }
    if (in[0]=='0' && (in[1]=='x'||in[1]=='X')) in += 2;
    for (int i = 0; i < 4 && in[i]; i++)
        out[i] = (char)tolower((unsigned char)in[i]);
    out[4] = 0;
}

static FILE *open_pci_ids(void) {
    const char *candidates[] = {
        g_pci_path[0] ? g_pci_path : NULL,
        "/usr/share/misc/pci.ids",
        "/usr/share/hwdata/pci.ids",
        "/usr/share/pci.ids",
        "data/pci.ids",
        NULL
    };
    for (int i = 0; candidates[i]; i++) {
        FILE *f = fopen(candidates[i], "r");
        if (f) return f;
    }
    return NULL;
}

const char *cyn_gpu_resolve_name(const char *vendor_id,
                                 const char *device_id) {
    /* CHANGE THIS LINE: 512 -> 1024 */
    static __thread char result[1024]; 
    result[0] = 0;

    char v[5] = {0}, d[5] = {0};
    norm_id(vendor_id, v);
    norm_id(device_id, d);
    if (!v[0] || !d[0]) return NULL;

    FILE *f = open_pci_ids();
    if (!f) return NULL;

    char line[512];
    int in_vendor = 0;
    char vendor_name[256] = {0};
    int found = 0;

    while (fgets(line, sizeof line, f)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        if (line[0] != '\t' && !isspace((unsigned char)line[0])) {
            char id[5] = {0};
            if (sscanf(line, "%4s", id) == 1) {
                for (int i = 0; id[i]; i++) id[i] = (char)tolower((unsigned char)id[i]);
                in_vendor = (strcmp(id, v) == 0);
                if (in_vendor) {
                    char *p = line + 4;
                    while (*p==' '||*p=='\t') p++;
                    p[strcspn(p,"\n")] = 0;
                    snprintf(vendor_name, sizeof vendor_name, "%.*s", (int)sizeof(vendor_name) - 1, p);
                }
            }
            continue;
        }

        if (!in_vendor) continue;

        if (line[0] == '\t' && line[1] != '\t') {
            char id[5] = {0};
            if (sscanf(line + 1, "%4s", id) == 1) {
                for (int i = 0; id[i]; i++) id[i] = (char)tolower((unsigned char)id[i]);
                if (strcmp(id, d) == 0) {
                    char *p = line + 5;
                    while (*p==' '||*p=='\t') p++;
                    p[strcspn(p,"\n")] = 0;
                    snprintf(result, sizeof result, "%.*s %.*s", 
                             (int)sizeof(vendor_name) - 1, vendor_name, 
                             (int)sizeof(line) - 1, p);
                    found = 1;
                    break;
                }
            }
        }
    }
    fclose(f);
    return found ? result : NULL;
}
