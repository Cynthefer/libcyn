# Changelog

All notable changes to this project will be documented in this file.

## [0.0.2] - 2026-11-27

This release focuses heavily on the Fedora/RPM packaging pipeline, resolving strict compiler warnings, and finalizing the zero-configuration global installation process.

**Author:** Ntando Khanyile <cynthefer@gmail.com>

### Build System & Installation
- **Zero-Configuration Setup:** The `Makefile` `install` target now automatically creates `/etc/ld.so.conf.d/libcyn.conf` and runs `ldconfig`. This permanently fixes the `error while loading shared libraries: libcyn.so.0` issue across all RPM-based and Debian-based systems.
- **Automatic Clang Integration:** Added automatic generation of `/etc/clang/global-config.cfg` during installation, allowing developers to compile with just `clang app.c -o app` without needing extra linker flags.
- **RPM Spec Fixes:** Corrected the `Release` tag formatting (`1%{?dist}`) and fixed the typo in the `License` tag (`BISD-3-Clause` to `BSD-3-Clause`).
- **Fedora Packaging:** Fixed the `fedpkg mockbuild` failure by renaming the template `newpackage.spec` to `libcyn.spec` and correcting `Source0` to point to the custom GitHub Actions tarball.

### Compiler & Stability Fixes
- **Eliminated All Truncation Warnings:** Resolved the stubborn `-Wformat-truncation` warnings in `src/gpu_pci.c` by increasing buffer sizes and utilizing precise `%.*s` formatting in `snprintf` calls.
- **Fixed Resolution Parsing:** Removed the deprecated `_res` struct in `src/network.c` (which caused compiler errors on newer glibc/musl) and replaced it with a safe, robust parser for `/etc/resolv.conf`.

### Hardware & Root Enhancements
- **`dmidecode` Execution:** `cyn_get_memory` now detects if the process is running as root. If so, it safely executes `dmidecode -t memory` to parse RAM Type, Speed, Form Factor, and CAS Latency.
- **Standardized Root Messages:** Unavailable root-only fields now cleanly return the exact string `"root needed"` instead of verbose error text.

### CI/CD & Portability
- **Valgrind CI Fix:** Added the `glibc-debug` package installation step for Arch Linux in the GitHub Actions matrix, resolving the Valgrind startup crash.
- **Sanitizer Matrix:** The CI now runs the test suite under AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), and Valgrind on Ubuntu, Fedora, and Arch.
- **Termux Compatibility:** Updated the `Makefile` to explicitly detect `llvm-ar` and `PREFIX=$PREFIX` to ensure seamless compilation on Android (Bionic libc).

### Documentation
- **Doxygen Integration:** Added comprehensive Doxygen comments to `include/cyn.h` and all internal structs, ensuring the GitHub Pages API reference generates correctly.
- **Man Page:** Updated the `cyn.3` man page with the new author details and corrected the date to November 2026.
