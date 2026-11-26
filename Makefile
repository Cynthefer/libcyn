# ==============================================================================
# libcyn Makefile
# Zero-configuration global installation. 
# Makes "clang app.c -o app" work out of the box.
# ==============================================================================

PREFIX  ?= /usr/local
DESTDIR ?= 
CC      ?= cc
AR      ?= $(shell command -v llvm-ar 2>/dev/null || command -v ar)

VERSION   = 0.1.0
SOVERSION = 0

CFLAGS  ?= -O2 -g
CFLAGS  += -Wall -Wextra -fPIC -fvisibility=hidden -Iinclude

SRC = src/cyn.c src/util.c src/user.c src/system.c src/network.c \
      src/memory.c src/storage.c src/cpu.c src/gpu.c src/gpu_pci.c \
      src/hardware.c src/json.c

OBJ = $(SRC:.c=.o)

all: libcyn.a libcyn.so.$(SOVERSION) libcyn.pc

libcyn.a: $(OBJ)
	@$(AR) rcs $@ $^

libcyn.so.$(SOVERSION): $(OBJ)
	@$(CC) -shared -Wl,-soname,libcyn.so.$(SOVERSION) -o $@ $^
	@ln -sf libcyn.so.$(SOVERSION) libcyn.so

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@

libcyn.pc: libcyn.pc.in
	@sed -e 's|@PREFIX@|$(PREFIX)|g' -e 's|@VERSION@|$(VERSION)|g' $< > $@

# --- Installation ---
install: all
	@echo "Installing libcyn..."
	@install -d $(DESTDIR)$(PREFIX)/include
	@install -m644 include/cyn.h $(DESTDIR)$(PREFIX)/include/
	
	@install -d $(DESTDIR)$(PREFIX)/lib
	@install -m644 libcyn.a $(DESTDIR)$(PREFIX)/lib/
	@install -m755 libcyn.so.$(SOVERSION) $(DESTDIR)$(PREFIX)/lib/
	@ln -sf libcyn.so.$(SOVERSION) $(DESTDIR)$(PREFIX)/lib/libcyn.so
	
	@install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	@install -m644 libcyn.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig/
	
	@install -d $(DESTDIR)$(PREFIX)/share/man/man3
	@install -m644 man/cyn.3 $(DESTDIR)$(PREFIX)/share/man/man3/

	# 1. Update OS dynamic linker (fixes "cannot open shared object file")
	@ldconfig 2>/dev/null || sudo ldconfig 2>/dev/null || true

	# 2. Tell Clang/GCC to look in /usr/local/lib and /usr/local/include globally
	@echo "Configuring global compiler paths..."
	@echo "$(PREFIX)/lib" | sudo tee /etc/ld.so.conf.d/libcyn.conf > /dev/null || true
	@echo "LDFLAGS=-L$(PREFIX)/lib" | sudo tee /etc/clang/global-config.cfg > /dev/null 2>&1 || true
	@echo "Configured! You can now use: clang myapp.c -o myapp"

uninstall:
	@rm -f $(DESTDIR)$(PREFIX)/include/cyn.h
	@rm -f $(DESTDIR)$(PREFIX)/lib/libcyn.a
	@rm -f $(DESTDIR)$(PREFIX)/lib/libcyn.so.$(SOVERSION)
	@rm -f $(DESTDIR)$(PREFIX)/lib/libcyn.so
	@rm -f $(DESTDIR)$(PREFIX)/lib/pkgconfig/libcyn.pc
	@rm -f $(DESTDIR)$(PREFIX)/share/man/man3/cyn.3
	@rm -f /etc/ld.so.conf.d/libcyn.conf
	@ldconfig 2>/dev/null || sudo ldconfig 2>/dev/null || true

clean:
	@rm -f $(OBJ) libcyn.a libcyn.so libcyn.so.$(SOVERSION) libcyn.pc test_cyn demo
