#ifndef CYN_H
#define CYN_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file cyn.h
 * @brief Public API for libcyn, a read-only Linux system information library.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define CYN_VERSION_MAJOR 0
#define CYN_VERSION_MINOR 0
#define CYN_VERSION_PATCH 1

#define CYN_API __attribute__((visibility("default")))

/* ========================================================================== */
/*                                  TYPES                                     */
/* ========================================================================== */

/**
 * @brief Status codes indicating the result of a data retrieval.
 */
typedef enum {
    CYN_OK = 0,         /**< The field was successfully populated. */
    CYN_UNAVAILABLE,    /**< Readable in principle, but not on this system / needs root. */
    CYN_ERROR           /**< A function call failed (e.g., out of memory). */
} cyn_status;

/**
 * @brief A wrapper for every single piece of data returned by libcyn.
 */
typedef struct {
    char        value[256]; /**< NUL-terminated string. Contains a reason if status != CYN_OK. */
    cyn_status  status;     /**< The status of this specific field. */
} cyn_field;

/**
 * @brief Convenience function to check if a field contains valid data.
 * @param f Pointer to the field.
 * @return 1 if status is CYN_OK, 0 otherwise.
 */
static inline int cyn_field_ok(const cyn_field *f) {
    return f && f->status == CYN_OK;
}

/* ---------- Sections (read-only views) ---------- */

/**
 * @brief User information.
 */
typedef struct {
    cyn_field username; /**< The login name of the current user. */
    cyn_field fullname; /**< The full name from the GECOS field. */
    cyn_field uid;      /**< The User ID (UID). */
    cyn_field gid;      /**< The Group ID (GID). */
    cyn_field home;     /**< The home directory path. */
    cyn_field shell;    /**< The default login shell. */
} cyn_user;

/**
 * @brief System information.
 */
typedef struct {
    cyn_field kernel;       /**< Kernel name (e.g., "Linux"). */
    cyn_field architecture; /**< Architecture (e.g., "x86_64"). */
    cyn_field release;      /**< Kernel release (e.g., "6.5.0-14-generic"). */
    cyn_field version;      /**< Kernel build version string. */
    cyn_field distro;       /**< Distribution name (e.g., "Fedora Linux 39"). */
} cyn_system;

/**
 * @brief Network information.
 */
typedef struct {
    cyn_field hostname;   /**< The system hostname. */
    cyn_field domain;     /**< The DNS domain from /etc/resolv.conf. */
    cyn_field interface;  /**< The primary network interface (e.g., "eth0"). */
    cyn_field local_ip;   /**< The local IPv4 address. */
    cyn_field public_ip;  /**< The public IPv4 address (requires fetch_public_ip). */
} cyn_network;

/**
 * @brief Memory information.
 */
typedef struct {
    cyn_field size;        /**< Total RAM (e.g., "16.0 GiB"). */
    cyn_field type;        /**< RAM Type (e.g., "DDR4"). Usually requires root. */
    cyn_field speed;       /**< RAM Speed (e.g., "3200 MT/s"). Usually requires root. */
    cyn_field form_factor; /**< Form factor (e.g., "DIMM"). Usually requires root. */
    cyn_field cas_latency; /**< CAS Latency. Usually requires root. */
} cyn_memory;

/**
 * @brief Storage information.
 */
typedef struct {
    cyn_field home_dir; /**< The mount point being checked (usually the home directory). */
    cyn_field total;    /**< Total capacity of the mount. */
    cyn_field used;     /**< Used capacity of the mount. */
    cyn_field free;     /**< Free capacity available to the user. */
} cyn_storage;

/**
 * @brief CPU information.
 */
typedef struct {
    cyn_field vendor; /**< The CPU manufacturer (e.g., "GenuineIntel"). */
    cyn_field model;  /**< The specific model name (e.g., "Intel(R) Core(TM) i7"). */
    cyn_field cores;  /**< The number of logical processing cores. */
} cyn_cpu;

/**
 * @brief GPU information.
 */
typedef struct {
    cyn_field vendor; /**< The PCI vendor ID (e.g., "0x10de") or resolved name. */
    cyn_field model;  /**< The PCI device ID (e.g., "0x2484") or resolved name. */
    cyn_field driver; /**< The active kernel driver (e.g., "amdgpu", "nvidia"). */
} cyn_gpu;

/**
 * @brief Hardware (DMI/SMBIOS) information.
 */
typedef struct {
    cyn_field brand;   /**< System vendor (e.g., "Dell Inc."). */
    cyn_field model;   /**< Product name (e.g., "XPS 15 9500"). */
    cyn_field type;    /**< Chassis type (numeric string). */
    cyn_field version; /**< Product version. */
} cyn_hardware;

/**
 * @brief Aggregate struct containing all system information.
 */
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

/**
 * @brief Configuration options passed to cyn_create().
 */
typedef struct {
    int fetch_public_ip;  /**< 1 to fetch via curl, 0 to skip (default). */
    int timeout_ms;       /**< Timeout for network requests (default: 2000). */
} cyn_options;

/** @brief Opaque context holding state and cache. */
typedef struct cyn_ctx cyn_ctx_t;

/* ========================================================================== */
/*                                LIFECYCLE                                   */
/* ========================================================================== */

/**
 * @brief Allocates and initializes a new context.
 * @param opts Configuration options, or NULL for defaults.
 * @return A new context, or NULL on out-of-memory.
 */
CYN_API cyn_ctx_t *cyn_create(const cyn_options *opts);

/**
 * @brief Frees the context and all cached data.
 * @param ctx The context to destroy. Safe to pass NULL.
 */
CYN_API void cyn_destroy(cyn_ctx_t *ctx);

/* ========================================================================== */
/*                                COLLECTION                                  */
/* ========================================================================== */

/**
 * @brief Collects all system information into a single struct.
 * @param ctx The active context.
 * @param out Pointer to the cyn_info struct to populate. Will be zeroed first.
 * @return CYN_OK if the collection routine ran successfully.
 */
CYN_API cyn_status cyn_collect(cyn_ctx_t *ctx, cyn_info *out);

/* Per-section collectors (independent, call any subset) */

/** @brief Collects user information. */
CYN_API void cyn_get_user    (cyn_ctx_t *, cyn_user     *);
/** @brief Collects OS and kernel information. */
CYN_API void cyn_get_system  (cyn_ctx_t *, cyn_system   *);
/** @brief Collects network and IP information. */
CYN_API void cyn_get_network (cyn_ctx_t *, cyn_network  *);
/** @brief Collects RAM information. */
CYN_API void cyn_get_memory  (cyn_ctx_t *, cyn_memory   *);
/** @brief Collects storage/mount information. */
CYN_API void cyn_get_storage (cyn_ctx_t *, cyn_storage  *);
/** @brief Collects CPU information. */
CYN_API void cyn_get_cpu     (cyn_ctx_t *, cyn_cpu      *);
/** @brief Collects GPU information. */
CYN_API void cyn_get_gpu     (cyn_ctx_t *, cyn_gpu      *);
/** @brief Collects hardware (DMI) information. */
CYN_API void cyn_get_hardware(cyn_ctx_t *, cyn_hardware *);

/* ========================================================================== */
/*                        SINGLE-FIELD CONVENIENCE                            */
/* ========================================================================== */

/** @brief Returns username, or NULL if unavailable. Do not free. */
CYN_API const char *cyn_user_username  (cyn_ctx_t *);
/** @brief Returns full name, or NULL if unavailable. Do not free. */
CYN_API const char *cyn_user_fullname  (cyn_ctx_t *);
/** @brief Returns kernel name, or NULL if unavailable. Do not free. */
CYN_API const char *cyn_system_kernel  (cyn_ctx_t *);
/** @brief Returns distro name, or NULL if unavailable. Do not free. */
CYN_API const char *cyn_system_distro  (cyn_ctx_t *);
/** @brief Returns hostname, or NULL if unavailable. Do not free. */
CYN_API const char *cyn_network_hostname(cyn_ctx_t *);
/** @brief Returns public IP, or NULL if unavailable. Do not free. */
CYN_API const char *cyn_network_public_ip(cyn_ctx_t *);
/** @brief Returns CPU model, or NULL if unavailable. Do not free. */
CYN_API const char *cyn_cpu_model      (cyn_ctx_t *);
/** @brief Returns hardware model, or NULL if unavailable. Do not free. */
CYN_API const char *cyn_hardware_model (cyn_ctx_t *);

/* ========================================================================== */
/*                                  MISC                                      */
/* ========================================================================== */

/** 
 * @brief Returns a thread-local string describing the last error.
 * @return A string that is never NULL.
 */
CYN_API const char *cyn_last_error(void);

/** 
 * @brief Converts a status enum to a human-readable string.
 * @param s The status.
 * @return "ok", "unavailable", or "error".
 */
CYN_API const char *cyn_status_str(cyn_status s);

/* ========================================================================== */
/*                            JSON SERIALIZATION                              */
/* ========================================================================== */

/**
 * @brief Serializes a cyn_info struct to a JSON string.
 * @param info The populated struct.
 * @return A heap-allocated JSON string. Must be freed with cyn_json_free(). Returns NULL on OOM.
 */
CYN_API char *cyn_to_json(const cyn_info *info);

/**
 * @brief Frees a JSON string returned by cyn_to_json().
 * @param json The string to free.
 */
CYN_API void  cyn_json_free(char *json);

/* ========================================================================== */
/*                            GPU RESOLUTION                                  */
/* ========================================================================== */

/**
 * @brief Resolves PCI IDs to human-readable names using pci.ids.
 * @param vendor_id The vendor hex string (e.g., "0x10de").
 * @param device_id The device hex string (e.g., "0x2484").
 * @return Pointer to internal buffer (valid until next call), or NULL if not found. Do not free.
 */
CYN_API const char *cyn_gpu_resolve_name(const char *vendor_id,
                                         const char *device_id);

/**
 * @brief Overrides the path to the pci.ids database.
 * @param path The absolute path to the file.
 */
CYN_API void cyn_set_pci_ids_path(const char *path);

#ifdef __cplusplus
}
#endif
#endif /* CYN_H */
