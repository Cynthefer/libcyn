# libcyn Design Document

This document outlines the architectural philosophy behind `libcyn`, the constraints we operate under, and the technical decisions made during development.

## 1. Core Philosophy

`libcyn` was built on three non-negotiable pillars:

### A. Unprivileged by Default
`libcyn` will **never** require `root`, `setuid`, `setgid`, or Linux capabilities. It operates strictly within the boundaries of a standard, unprivileged user session. If a piece of system information is fundamentally locked behind root privileges (such as DMI/SMBIOS memory timings), `libcyn` will report it as `CYN_UNAVAILABLE` rather than trying to bypass the kernel's security model.

### B. Zero Side Effects
A library should not dictate how an application behaves. Therefore, `libcyn`:
- Does **not** print to stdout/stderr.
- Does **not** write to disk, cache files, or temporary directories.
- Does **not** spawn shells (e.g., `system()`, `popen()`). *(The only exception is an explicit, opt-in `curl` call for the public IP address).*

### C. Trinary Truth
In C, an empty string (`""`) is not the same as "not found," and "not found" is not the same as "permission denied." `libcyn` uses a trinary status system (`CYN_OK`, `CYN_UNAVAILABLE`, `CYN_ERROR`) attached to every single field so callers can present accurate information to their users.

---

## 2. Architecture & Memory Model

### Opaque Context (`cyn_ctx_t`)
The core state of the library is hidden behind an opaque pointer. This allows us to add new configuration options (like timeouts or custom paths) in the future without breaking the ABI. 

The context holds the `cyn_options` and a lazy-loaded cache used by the convenience accessors (e.g., `cyn_system_distro()`).

### Fixed-Size Fields (`cyn_field`)
Every data point is stored in a fixed-size `char value[256]` array rather than heap-allocated strings.
- **Why?** It drastically simplifies memory management. Callers never have to `free()` individual strings. It avoids memory fragmentation and guarantees that a `cyn_info` struct can be safely passed by value or copied.
- **The Trade-off:** Strings longer than 255 bytes are truncated. In the context of system information (paths, OS names, CPU models), 255 bytes is more than sufficient.

### Thread Safety
- **Contexts are NOT thread-safe.** Do not share a single `cyn_ctx_t` across threads. Create one per thread.
- **Error handling is thread-safe.** `cyn_last_error()` uses `__thread` (Thread-Local Storage) so that concurrent failures do not overwrite each other's error messages.
- **GPU resolution is thread-safe.** `cyn_gpu_resolve_name()` uses a `__thread` static buffer to return strings without requiring the caller to `free()` them.

---

## 3. Data Sources & Root Restrictions

`libcyn` reads directly from the kernel's virtual filesystem (`/proc`, `/sys`) and standard libc APIs. We avoid parsing the output of shell commands wherever possible (with the exception of `pci.ids`).

| Feature | Source | Root Required? | Reasoning |
|---------|--------|----------------|-----------|
| User/Group | `getpwuid()` | No | Standard libc. |
| Kernel/Arch | `uname()` | No | Standard libc. |
| Distro | `/etc/os-release` | No | World-readable standard. |
| Network IPs | `getifaddrs()` | No | Standard libc. |
| Domain Name | `/etc/resolv.conf` | No | World-readable. |
| RAM Size | `/proc/meminfo` | No | Kernel exposes total RAM. |
| Storage | `statvfs()` | No | Standard libc. |
| CPU Model | `/proc/cpuinfo` | No | Kernel exposes this globally. |
| GPU | `/sys/class/drm/` | No | DRM subsystem exposes PCI IDs. |
| Hardware | `/sys/class/dmi/id/` | No | Usually `0444` (world-readable). |
| **RAM Speed/CAS** | DMI SMBIOS | **Yes** | Kernel restricts `/dev/mem` to root. |
| **Public IP** | `curl api.ipify.org` | No | Requires outbound HTTP (opt-in). |

### Why not use `dmidecode`?
Tools like `dmidecode` read raw memory (`/dev/mem`) to parse SMBIOS tables. This strictly requires root. We do not fork `dmidecode` because it violates our "No Privilege Escalation" and "No Shelling Out" rules.

---

## 4. The JSON Serializer

We include a built-in JSON serializer (`cyn_to_json`). This was added because 90% of consumers of a system info library want to pipe the data into a web dashboard, log aggregator, or TUI. 

We use `open_memstream` on Linux (glibc/musl) to build the JSON dynamically, ensuring we don't have to guess the final string size. The string is allocated on the heap, and the caller must free it using `cyn_json_free()`.

---

## 5. GPU Resolution & `pci.ids`

Reading `/sys/class/drm/` only provides hex IDs (e.g., `0x10de`). To translate this into a human-readable name, `libcyn` parses `pci.ids`. 

- **We do not bundle `pci.ids`.** It is hundreds of kilobytes and updates frequently. We rely on the system's package manager (`pciutils` or `hwdata`) to provide it.
- **Fallback:** If `pci.ids` is missing, the library gracefully returns the raw hex IDs. It does not crash.
- **Performance:** The file is parsed line-by-line on demand. It is not cached in memory to keep the library footprint small.

---

## 6. Platform Support

### Linux (glibc/musl)
This is the primary target. Everything works as documented.

### Termux / Android (Bionic libc)
Termux is fully supported, but requires specific handling:
- **Pathing:** Termux uses a non-standard prefix (`/data/data/com.termux/files/usr`). The `Makefile` respects the `$PREFIX` environment variable.
- **Bionic Limitations:** Historically, Android's Bionic libc lacked `open_memstream`. Recent versions support it, but if compiling on older Android versions, the JSON serializer may fail to compile.
- **Permissions:** Android restricts access to `/sys/class/drm/` and `/sys/class/dmi/` for unprivileged apps. Fields from these directories will gracefully return `CYN_UNAVAILABLE`.

### MacOS / Windows
Explicitly **not supported**. The library relies heavily on Linux-specific virtual filesystems (`/proc`, `/sys`).

---

## 7. Build Systems

The library provides two parallel build systems:
1. **`Makefile`**: Traditional, zero-dependency build. Ideal for Termux and minimal containers.
2. **`CMakeLists.txt`**: Standard modern build, used by downstream projects that integrate `libcyn` via `add_subdirectory()`.

Both build systems generate `libcyn.a` (static) and `libcyn.so` (shared).
