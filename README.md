# libcyn

[![Docs](https://img.shields.io/badge/docs-latest-blue)](https://cynthefer.github.io/libcyn/)

libcyn is a robust, dependency-free C library for querying Linux system information — users, hardware, network, storage, and OS details — entirely without privilege escalation.

Unlike command-line tools that format text for humans, libcyn provides raw, structured data. It outputs nothing. It spawns no shells. It requires no root privileges. You ask for a struct, and it fills it with validated, zero-terminated strings. What you do with that data (print it, serialize it, send it over a socket) is entirely up to you.

---

## Table of Contents

1. Key Features & Engineering Advantages
2. Why libcyn? (vs. Traditional Tools)
3. Quick Start Example
4. Linking: Static vs. Shared
5. The "No Privilege Escalation" Guarantee
6. Data Source Reference
7. Advanced Usage: JSON & GPU Resolution
8. Requirements & Portability
9. Building & Installing
10. Documentation & API Reference

---

## 1. Key Features & Engineering Advantages

### Scalable & High-Performance
Traditional system tools (neofetch, inxi) spawn multiple processes (fork, exec), pipe text through awk and grep, and parse strings. This is disastrously slow for loops, daemons, or high-frequency monitoring. 
libcyn does none of this. It bypasses the shell entirely, reading directly from /proc, /sys, and standard libc APIs. It executes in microseconds and has virtually zero CPU overhead, making it safe to call in tight loops or performance-critical daemons.

### Unprivileged by Design (Zero Escalation)
Security is non-negotiable. libcyn requires no sudo, setuid, or Linux capabilities. Instead of trying to bypass the kernel's security model to read restricted SMBIOS/DMI memory tables, libcyn reports those specific fields as CYN_UNAVAILABLE along with a human-readable reason. Your software runs perfectly as a standard user.

### Ultra-Portable
Written in strict C99 with zero external runtime dependencies. It compiles flawlessly on:
- Linux: glibc and musl (Alpine)
- Architecture: x86_64, ARM64 (aarch64), and ARMv7
- Termux (Android): Fully supported. The library understands Termux's unique prefix (/data/data/com.termux/files/usr) and Bionic libc limitations.

### Trinary Truth & Data Integrity
In C, an empty string ("") is not the same as "not found," and "not found" is not the same as "permission denied." Every field returned by libcyn is wrapped in a cyn_field struct containing a status (CYN_OK, CYN_UNAVAILABLE, CYN_ERROR). You will never have to guess why a string is empty.

### Zero Side Effects
A library should not dictate how an application behaves. libcyn does not print to stdout, write cache files to disk, or spawn shells. (The only exception is an explicit, opt-in curl call if you request the public IP address).

### Built-in Extras
- JSON Serializer: Generate formatted JSON instantly with cyn_to_json().
- GPU Resolution: Automatically translates raw PCI hex IDs (e.g., 0x10de) into human-readable names by parsing pci.ids.
- Dual Build: Compiles to both libcyn.a (for standalone binaries) and libcyn.so (for system packages).

---

## 2. Why libcyn? (vs. Traditional Tools)

If you have ever tried to build a dashboard, a monitoring agent, or a TUI in C, you know the pain of gathering system information. The traditional approach involves shelling out to tools like neofetch, lspci, or uname, and then using regex to parse their text output. 

| Feature | Traditional Tools (neofetch, inxi) | libcyn |
| :--- | :--- | :--- |
| Interface | Text output parsed via regex / string splitting. | Structured C Structs. Type-safe, validated. |
| Overhead | Spawns 5-10 separate processes (fork, exec, pipe). | Zero process spawning. Direct /proc access. |
| Security | Requires sudo for full data. | Zero privilege escalation. Runs as user. |
| Scalability | Terrible for tight loops. Text parsing is slow. | Blazing fast. Microsecond data retrieval. |
| Portability | Breaks if awk is missing or formats change. | Deterministic. Works on glibc, musl, and Termux. |

---

## 3. Quick Start Example

Here is how you fetch the operating system's distro name and total RAM:

    #include <cyn.h>
    #include <stdio.h>

    int main(void) {
        /* Pass NULL for defaults. */
        cyn_ctx_t *ctx = cyn_create(NULL);
        if (!ctx) {
            fprintf(stderr, "Failed to create context\n");
            return 1;
        }

        cyn_info info;
        /* Collects all sections: user, system, network, memory, etc. */
        if (cyn_collect(ctx, &info) == CYN_OK) {
            
            /* 1. Check if the field is available */
            if (cyn_field_ok(&info.system.distro)) {
                printf("Distro: %s\n", info.system.distro.value);
            } else {
                printf("Distro: Unknown (%s)\n", info.system.distro.value);
            }

            if (cyn_field_ok(&info.memory.size)) {
                printf("RAM: %s\n", info.memory.size.value);
            }

            /* 2. Observe an unavailable field and its reason */
            if (!cyn_field_ok(&info.memory.speed)) {
                /* Outputs: Memory Speed: requires root (dmidecode) */
                printf("Memory Speed: %s\n", info.memory.speed.value);
            }
        }

        cyn_destroy(ctx);
        return 0;
    }

---

## 4. Linking: Static vs. Shared

libcyn builds both a static (libcyn.a) and a shared (libcyn.so.0) library. You get to choose how you want to link your application.

### When to use Shared Linking (Default)
Use this if you are distributing your software as a package (.deb, .rpm, AUR). The binary stays small, and if libcyn is updated via the system package manager, your software automatically uses the patched version.

    cc myapp.c $(pkg-config --cflags --libs libcyn) -o myapp

### When to use Static Linking
Use this if you want to build a single, standalone executable that you can drop onto any compatible Linux server. The libcyn code is physically copied into your binary, meaning the target machine does not need libcyn installed to run it.

    cc myapp.c -I/usr/local/include /usr/local/lib/libcyn.a -o myapp

---

## 5. The "No Privilege Escalation" Guarantee

Security and stability were the primary design goals. libcyn does not require sudo, setuid, setgid, or Linux Capabilities. 

If a piece of information is locked behind root permissions (typically SMBIOS/DMI memory timing data), libcyn will not attempt to bypass the kernel's security model. Instead, it populates that specific field's .value with an explanation (e.g., "requires root (dmidecode)") and sets its .status to CYN_UNAVAILABLE.

We do not read /dev/mem, we do not spawn sudo, and we do not use system() or popen().

---

## 6. Data Source Reference

Here is a detailed breakdown of exactly where libcyn pulls its data from, and what happens if the file or API is unavailable.

| Section | Field | Source | Root Required? |
| :--- | :--- | :--- | :--- |
| User | username, uid, gid, home, shell | getpwuid() | No |
| | fullname | getpwuid() (GECOS field) | No |
| System | kernel, architecture, release, version | uname() syscall | No |
| | distro | /etc/os-release | No |
| Network | hostname | gethostname() | No |
| | domain | /etc/resolv.conf | No |
| | interface, local_ip | getifaddrs() (skips loopback) | No |
| | public_ip | curl https://api.ipify.org | No (Requires internet) |
| Memory | size | /proc/meminfo | No |
| | type, speed, form_factor, cas_latency | DMI SMBIOS tables | Yes (Marked Unavailable) |
| Storage | home_dir | getpwuid() | No |
| | total, used, free | statvfs() on home_dir | No |
| CPU | vendor, model, cores | /proc/cpuinfo | No |
| GPU | vendor, model (PCI IDs) | /sys/class/drm/card*/device/ | No |
| | driver | /sys/class/drm/card*/device/uevent | No |
| Hardware | brand, model, type, version | /sys/class/dmi/id/* (DMI IDs) | No (Usually 0444) |

---

## 7. Advanced Usage: JSON & GPU Resolution

### JSON Serialization
If you need to send system info over a network or log it to a file, use the built-in serializer. This allocates memory which you must free.

    #include <cyn.h>
    #include <stdio.h>

    int main(void) {
        cyn_ctx_t *ctx = cyn_create(NULL);
        cyn_info info;
        cyn_collect(ctx, &info);

        char *json = cyn_to_json(&info);
        if (json) {
            printf("%s\n", json);
            cyn_json_free(json); /* Must free! */
        }

        cyn_destroy(ctx);
        return 0;
    }

### GPU Name Resolution
By default, /sys/class/drm only provides hex IDs (e.g., Vendor: 0x10de, Device: 0x2484). If you have the pci.ids file installed on your system (typically provided by the pciutils or hwdata packages), libcyn will automatically attempt to resolve these IDs into human-readable strings.

If you want to resolve them manually, or override the default search paths:

    /* Optional: set a custom path to pci.ids */
    cyn_set_pci_ids_path("/usr/share/misc/pci.ids");

    const char *name = cyn_gpu_resolve_name("0x10de", "0x2484");
    if (name) {
        printf("Resolved GPU: %s\n", name); 
        /* Output: NVIDIA Corporation GA104 [GeForce RTX 3070] */
    }

---

## 8. Requirements & Portability

libcyn is designed to be highly portable across the Linux ecosystem.

- Operating System: Linux (glibc or musl based). 
- Architecture: Works on x86_64, ARM64 (aarch64), and ARMv7.
- Termux (Android): Fully supported. The library understands Termux's unique prefix (/data/data/com.termux/files/usr) and Bionic libc limitations.
- Compiler: C99 compliant (GCC or Clang).
- Build Tools: make or cmake (>= 3.15).
- Optional Dependencies:
  - curl (Only if fetch_public_ip is enabled in options).
  - pci.ids (Usually at /usr/share/misc/pci.ids, for GPU name resolution).

---

## 9. Building & Installing

### 1. Using Make (Standard)

    make
    make check       # Compiles and runs the test suite
    sudo make install

To stage an installation for a package manager:

    make install DESTDIR=/tmp/libcyn-pkg PREFIX=/usr

### 2. Using CMake

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j
    ctest --test-dir build --output-on-failure
    sudo cmake --install build

### 3. Uninstalling

Makefile:

    sudo make uninstall

CMake:

    cd build && sudo cmake --build . --target uninstall

---

## 10. Documentation & API Reference

For a deep dive into the architecture, memory ownership rules, and threading safety, see the dedicated documentation files in the docs/ folder:

- API Reference: See docs/API.md for full struct layouts, functions, and threading rules.
- Design Philosophy: See docs/DESIGN.md for why certain fields require root and how statuses are handled.
- Building Guide: See docs/BUILDING.md for sanitizer builds and packaging instructions.
- Man Page: man cyn (installed via make install)

## License

This project is licensed under the BSD 3-Clause License. See the LICENSE file for details.
