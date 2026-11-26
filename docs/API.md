
---

## Final Makefile tweak

Add the two new sources and the pkg-config rule:

```makefile
VERSION   = 0.1.0
SRC = src/cyn.c src/util.c src/user.c src/system.c src/network.c \
      src/memory.c src/storage.c src/cpu.c src/gpu.c src/gpu_pci.c \
      src/hardware.c src/json.c