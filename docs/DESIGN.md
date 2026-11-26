
---

## `docs/DESIGN.md`

markdown
# libcyn — Design

## Goals

1. **No privilege escalation.**
   Every field comes from a world-readable source (`/proc`, `/sys`,
   `/etc/os-release`, `getpwuid`, `getifaddrs`, `uname`, `statvfs`,
   `pci.ids`). If a field would need root, we mark it unavailable.
2. **No side effects beyond the explicit.**
   The library never writes to disk, never prints, never spawns shells.
   The only outbound action is the opt-in public-IP fetch.
3. **Per-field truthfulness.**
   "Unavailable" is not the same as "empty string" — callers get a status
   per field.

## Why an opaque context?

`cyn_ctx_t` holds options and the cache used by convenience accessors.
Keeping it opaque lets us add fields without breaking the ABI.

## Why a fixed-size `cyn_field.value`?

Simplicity and no allocation on the hot path. 256 bytes is enough for
every field we emit (paths, distro names, hardware strings). If a value
exceeds this, it is truncated — by design, not by accident.

## Status model
