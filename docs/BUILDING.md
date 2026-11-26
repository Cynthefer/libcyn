# Building libcyn

## Requirements

- Linux (any modern distro)
- A C99 compiler (gcc or clang)
- `make`
- Optional: `curl` (for `fetch_public_ip`)
- Optional: `pci.ids` from the `pciutils`/`hwdata` package (for GPU names)

## Build

```sh
make
```

## Uninstall

### Makefile
```sh
sudo make uninstall
```