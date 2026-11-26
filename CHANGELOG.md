# Changelog

All notable changes to this project will be documented in this file.

## [0.1.0] - 2026-11-26

This is the initial release of `libcyn`, a robust, dependency-free C library for querying Linux system information without privilege escalation. 

**Author:** Ntando Khanyile <cynthefer@gmail.com>

### Documentation & Ecosystem
- **Doxygen Integration:** Added comprehensive Doxygen comments to `include/cyn.h`, structs, and internal macros. Set up a GitHub Actions workflow to automatically build and host the API reference on GitHub Pages.
- **Rich README:** Completely rewrote `README.md` to emphasize engineering advantages, scalability, zero-overhead, and the "No Privilege Escalation" guarantee.
- **Developer Guides:** Added `docs/API.md`, `docs/DESIGN.md`, and `docs/BUILDING.md` for deep dives into architecture, memory models, and build instructions.
- **License:** Officially adopted the BSD 3-Clause License.

### Zero-Configuration Install (`Makefile`)
- **Global Clang/GCC Config:** The `install` target now automatically drops a configuration file into `/etc/clang/global-config.cfg`. You can now compile applications without any linker flags (e.g., `clang cyn.c -o cyn` just works out of the box).
- **Linker Cache (`ldconfig`):** Added automatic `ldconfig` execution and `/etc/ld.so.conf.d/libcyn.conf` generation to permanently fix the `error while loading shared libraries: libcyn.so.0` issue on Fedora, Debian, and Arch.
- **Uninstall Cleanup:** The `make uninstall` command now correctly removes all system-wide configuration files and refreshes the dynamic linker cache.

### Stability & Compiler Fixes
- **Strict Memory Safety:** Fixed Valgrind "uninitialized value" warnings in `gpu_pci.c`. Arrays are now explicitly zero-initialized (`{0}`) before parsing.
- **Truncation Warnings Fixed:** Implemented `%.*s` precision formatting in `snprintf` calls and increased buffer sizes in `gpu_pci.c` (up to 1024 bytes), completely eliminating Clang `-Wformat-truncation` warnings.
- **Resolver State Fix:** Removed the deprecated `_res` struct parsing in `network.c`. Replaced it with a robust, cross-platform parser for `/etc/resolv.conf` to fetch the domain name safely.

### Hardware & Root Enhancements
- **`dmidecode` Parsing:** The `cyn_get_memory` function now detects if the user has root privileges. If so, it safely executes `dmidecode -t memory` via our internal execution wrapper (no shell injection) to fetch RAM Type, Speed, Form Factor, and CAS Latency.
- **Standardized Messages:** Unavailable root-only fields now cleanly return the reason `"root needed"` instead of a verbose error string.

### CI/CD & Portability
- **Cross-Distro Testing:** GitHub Actions now runs a matrix build across **Ubuntu 24.04**, **Fedora (latest)**, and **Arch Linux** using both `gcc` and `clang`.
- **Sanitizers:** Every CI run now compiles and tests the code with `AddressSanitizer` (ASan) and `UndefinedBehaviorSanitizer` (UBSan) to guarantee memory safety.
- **Valgrind Integration:** The CI matrix runs the test suite under Valgrind (with Arch-specific `glibc-debug` fixes applied) to ensure zero memory leaks.
- **Termux Support:** Updated the `Makefile` and `network.c` to seamlessly support compilation on Android via Termux (Bionic libc).
