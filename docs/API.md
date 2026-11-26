# libcyn API Reference

All public symbols are declared in `<cyn.h>` and are exported with `CYN_API` (the library is compiled with `-fvisibility=hidden`).

## Quick Reference

- [Status System](#status-system)
- [Data Structures](#data-structures)
- [Context Lifecycle](#context-lifecycle)
- [Collection Functions](#collection-functions)
- [Convenience Accessors](#convenience-accessors)
- [JSON Serialization](#json-serialization)
- [GPU Resolution](#gpu-resolution)
- [Error Handling](#error-handling)
- [Thread Safety](#thread-safety)

---

## Status System

`libcyn` uses a trinary status system to distinguish between data that is successfully read, data that is fundamentally unavailable on the system, and errors.

```c
typedef enum {
    CYN_OK = 0,
    CYN_UNAVAILABLE,
    CYN_ERROR
} cyn_status;
```
