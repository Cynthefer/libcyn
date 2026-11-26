# libcyn

[![Docs](https://img.shields.io/badge/docs-latest-blue)](https://cynthefer.github.io/libcyn/)

`libcyn` is a small, dependency-free C library for **reading Linux system
information** — user, system, network, memory, storage, CPU, GPU, and
hardware — **without privilege escalation**.

It does **not** print anything. It exposes structured fields that callers
format however they like (plain text, JSON, TUI, network protocol, …).

## Requirements

- Linux (any modern distro, glibc or musl)
- C99 compiler
- **Optional**: `curl` for public IP lookup
- **Optional**: `pci.ids` (from `pciutils` or `hwdata`) for GPU names
- **Never**: root, setuid, capabilities

## Features

- No root required. Fields that genuinely need root are reported as
  `CYN_UNAVAILABLE` — never bypassed.
- Per-field status: distinguish *empty* from *unreadable* from *unavailable*.
- Zero runtime dependencies (libc only).
- Optional `pci.ids` lookup for friendly GPU names.
- Optional JSON serializer (`cyn_to_json`) — still no I/O.
- Public-IP lookup is **opt-in** and outbound-only.

## Quick start

```c
#include <cyn.h>
#include <stdio.h>

int main(void) {
    cyn_ctx_t *ctx = cyn_create(NULL);
    cyn_info info;
    cyn_collect(ctx, &info);

    if (cyn_field_ok(&info.system.distro))
        printf("distro: %s\n", info.system.distro.value);

    cyn_destroy(ctx);
}
```

## Documentation

- **Man page**: `man cyn` after `make install`
- **Reference**: [`docs/API.md`](docs/API.md)
- **Design**: [`docs/DESIGN.md`](docs/DESIGN.md)
- **Building**: [`docs/BUILDING.md`](docs/BUILDING.md)

## Building with CMake (alternative)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
sudo cmake --install build
```
