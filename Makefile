# ==============================================================================
# libcyn Makefile
# Zero-configuration global installation.
# ==============================================================================

PREFIX  ?= /usr
LIBDIR  ?= $(PREFIX)/lib
DESTDIR ?= 
CC      ?= cc
AR      ?= $(shell command -v llvm-ar 2>/dev/null || command -v ar)

VERSION   = 0.0.2
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
	@sed -e 's|@PREFIX@|$(PREFIX)|g' \
	     -e 's|@LIBDIR@|$(LIBDIR)|g' \
	     -e 's|@VERSION@|$(VERSION)|g' $< > $@

# --- Installation ---
install: all
	@echo "Installing libcyn to $(DESTDIR)$(PREFIX)..."
	
	@echo "# 1. Copy Headers"
	@install -d $(DESTDIR)$(PREFIX)/include
	@install -m644 include/cyn.h $(DESTDIR)$(PREFIX)/include/
	
	@echo "# 2. Copy Libraries"
	@install -d $(DESTDIR)$(LIBDIR)
	@install -m644 libcyn.a $(DESTDIR)$(LIBDIR)/
	@install -m755 libcyn.so.$(SOVERSION) $(DESTDIR)$(LIBDIR)/
	@ln -sf libcyn.so.$(SOVERSION) $(DESTDIR)$(LIBDIR)/libcyn.so
	
	@echo "# 3. Copy pkg-config & Man page"
	@install -d $(DESTDIR)$(LIBDIR)/pkgconfig
	@install -m644 libcyn.pc $(DESTDIR)$(LIBDIR)/pkgconfig/
	@install -d $(DESTDIR)$(PREFIX)/share/man/man3
	@install -m644 man/cyn.3 $(DESTDIR)$(PREFIX)/share/man/man3/

	@echo "# 4. System Configuration (Skip if packaging)"
	@if [ -z "$(DESTDIR)" ]; then \
		echo "Configuring global system paths..."; \
		echo "$(LIBDIR)" | sudo tee /etc/ld.so.conf.d/libcyn.conf > /dev/null; \
		sudo ldconfig; \
		sudo mkdir -p /etc/clang; \
		echo "LDFLAGS=-L$(LIBDIR) -lcyn" | sudo tee /etc/clang/global-config.cfg > /dev/null; \
		echo "Done! You can now compile using: clang myapp.c -o myapp"; \
	fi

uninstall:
	@echo "Uninstalling libcyn..."
	@rm -f $(DESTDIR)$(PREFIX)/include/cyn.h
	@rm -f $(DESTDIR)$(LIBDIR)/libcyn.a
	@rm -f $(DESTDIR)$(LIBDIR)/libcyn.so.$(SOVERSION)
	@rm -f $(DESTDIR)$(LIBDIR)/libcyn.so
	@rm -f $(DESTDIR)$(LIBDIR)/pkgconfig/libcyn.pc
	@rm -f $(DESTDIR)$(PREFIX)/share/man/man3/cyn.3
	@if [ -z "$(DESTDIR)" ]; then \
		sudo rm -f /etc/ld.so.conf.d/libcyn.conf; \
		sudo rm -f /etc/clang/global-config.cfg; \
		sudo ldconfig; \
	fi
	@echo "Uninstall complete."

check: all
	@$(CC) $(CFLAGS) tests/test_cyn.c libcyn.a -o test_cyn
	@./test_cyn

clean:
	@rm -f $(OBJ) libcyn.a libcyn.so libcyn.so.$(SOVERSION) libcyn.pc test_cyn demo