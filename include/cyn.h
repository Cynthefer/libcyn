#ifndef CYN_H
#define CYN_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CYN_VERSION_MAJOR 0
#define CYN_VERSION_MINOR 0
#define CYN_VERSION_PATCH 1

#define CYN_API __attribute__((visibility("default")))

/* ---------- Status ---------- */
typedef enum {
    CYN_OK = 0,
    CYN_UNAVAILABLE,   /* readable in principle, but not on this system / needs root */
    CYN_ERROR
} cyn_status;

/* ---------- Field ---------- */
typedef struct {
    char        value[256];
    cyn_status  status;
} cyn_field;

/* Convenience: is this field actually populated? */
static inline int cyn_field_ok(const cyn_field *f) {
    return f && f->status == CYN_OK;
}

/* ---------- Sections (read-only views) ---------- */
typedef struct {
    cyn_field username;
    cyn_field fullname;
    cyn_field uid;
    cyn_field gid;
    cyn_field home;
    cyn_field shell;
} cyn_user;

typedef struct {
    cyn_field kernel;
    cyn_field architecture;
    cyn_field release;
    cyn_field version;
    cyn_field distro;
} cyn_system;

typedef struct {
    cyn_field hostname;
    cyn_field domain;
    cyn_field interface;
    cyn_field local_ip;
    cyn_field public_ip;
} cyn_network;

typedef struct {
    cyn_field size;
    cyn_field type;
    cyn_field speed;
    cyn_field form_factor;
    cyn_field cas_latency;
} cyn_memory;

typedef struct {
    cyn_field home_dir;
    cyn_field total;
    cyn_field used;
    cyn_field free;
} cyn_storage;

typedef struct {
    cyn_field vendor;
    cyn_field model;
    cyn_field cores;
} cyn_cpu;

typedef struct {
    cyn_field vendor;
    cyn_field model;
    cyn_field driver;
} cyn_gpu;

typedef struct {
    cyn_field brand;
    cyn_field model;
    cyn_field type;
    cyn_field version;
} cyn_hardware;

/* ---------- Aggregate ---------- */
typedef struct {
    cyn_user     user;
    cyn_system   system;
    cyn_network  network;
    cyn_memory   memory;
    cyn_storage  storage;
    cyn_cpu      cpu;
    cyn_gpu      gpu;
    cyn_hardware hardware;
} cyn_info;

/* ---------- Options ---------- */
typedef struct {
    int fetch_public_ip;  /* 0 = skip */
    int timeout_ms;       /* 0 = 2000 */
} cyn_options;

/* ---------- Context ---------- */
typedef struct cyn_ctx cyn_ctx_t;

/* ---------- Lifecycle ---------- */
CYN_API cyn_ctx_t *cyn_create(const cyn_options *opts);
CYN_API void       cyn_destroy(cyn_ctx_t *ctx);

/* ---------- Full collection ---------- */
/* Fills `out`; caller owns it. Returns CYN_OK if collection ran.
 * Check individual cyn_field.status values for per-field success. */
CYN_API cyn_status cyn_collect(cyn_ctx_t *ctx, cyn_info *out);

/* ---------- Per-section (independent, call any subset) ---------- */
CYN_API void cyn_get_user    (cyn_ctx_t *, cyn_user     *);
CYN_API void cyn_get_system  (cyn_ctx_t *, cyn_system   *);
CYN_API void cyn_get_network (cyn_ctx_t *, cyn_network  *);
CYN_API void cyn_get_memory  (cyn_ctx_t *, cyn_memory   *);
CYN_API void cyn_get_storage (cyn_ctx_t *, cyn_storage  *);
CYN_API void cyn_get_cpu     (cyn_ctx_t *, cyn_cpu      *);
CYN_API void cyn_get_gpu     (cyn_ctx_t *, cyn_gpu      *);
CYN_API void cyn_get_hardware(cyn_ctx_t *, cyn_hardware *);

/* ---------- Single-field convenience ---------- */
/* Returns pointer to internal value string, or NULL if unavailable.
 * Do NOT free the result. */
CYN_API const char *cyn_user_username  (cyn_ctx_t *);
CYN_API const char *cyn_user_fullname  (cyn_ctx_t *);
CYN_API const char *cyn_system_kernel  (cyn_ctx_t *);
CYN_API const char *cyn_system_distro  (cyn_ctx_t *);
CYN_API const char *cyn_network_hostname(cyn_ctx_t *);
CYN_API const char *cyn_network_public_ip(cyn_ctx_t *);
CYN_API const char *cyn_cpu_model      (cyn_ctx_t *);
CYN_API const char *cyn_hardware_model (cyn_ctx_t *);

/* ---------- Misc ---------- */
CYN_API const char *cyn_last_error(void);
CYN_API const char *cyn_status_str(cyn_status s);

#ifdef __cplusplus
}
#endif
#endif

/* ---------- JSON serialization (no I/O in the library) ---------- */
/* Returns a heap-allocated JSON string, or NULL on OOM.
 * Caller must free with cyn_json_free(). */
CYN_API char *cyn_to_json(const cyn_info *info);
CYN_API void  cyn_json_free(char *json);

/* ---------- GPU name resolution ---------- */
/* Resolve PCI IDs to human-readable names using pci.ids.
 * Returns pointer to internal buffer (valid until next call), or NULL.
 * `vendor_id` and `device_id` are the hex strings from cyn_gpu
 * (e.g. "0x10de", "0x2484"). */
CYN_API const char *cyn_gpu_resolve_name(const char *vendor_id,
                                         const char *device_id);

/* Optionally override the pci.ids path (default: /usr/share/misc/pci.ids,
 * then /usr/share/hwdata/pci.ids, then ./data/pci.ids). */
CYN_API void cyn_set_pci_ids_path(const char *path);