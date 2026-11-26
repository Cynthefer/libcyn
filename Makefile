CC      ?= cc
CFLAGS  ?= -O2 -g
CFLAGS  += -Wall -Wextra -fPIC -fvisibility=hidden -Iinclude
PREFIX  ?= /usr/local
SOVERSION = 0
VERSION = 0.0.1

libcyn.pc: libcyn.pc.in
	sed -e 's|@PREFIX@|$(PREFIX)|g' \
	    -e 's|@VERSION@|$(VERSION)|g' \
	    libcyn.pc.in > libcyn.pc

SRC = src/cyn.c src/util.c src/user.c src/system.c src/network.c \
      src/memory.c src/storage.c src/cpu.c src/gpu.c src/hardware.c
OBJ = $(SRC:.c=.o)

all: libcyn.a libcyn.so

libcyn.a: $(OBJ)
	$(AR) rcs $@ $^

libcyn.so: $(OBJ)
	$(CC) -shared -Wl,-soname,libcyn.so.$(SOVERSION) -o $@ $^
	ln -sf libcyn.so.$(SOVERSION) libcyn.so

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: all libcyn.pc
	install -d $(DESTDIR)$(PREFIX)/include $(DESTDIR)$(PREFIX)/lib
	install -m644 include/cyn.h $(DESTDIR)$(PREFIX)/include/
	install -m644 libcyn.a $(DESTDIR)$(PREFIX)/lib/
	install -m755 libcyn.so.$(SOVERSION) $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/share/man/man3
	install -m644 man/cyn.3 $(DESTDIR)$(PREFIX)/share/man/man3
	install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	install -m644 libcyn.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig/
	ln -sf libcyn.so.$(SOVERSION) $(DESTDIR)$(PREFIX)/lib/libcyn.so

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/include/cyn.h
	rm -f $(DESTDIR)$(PREFIX)/lib/libcyn.a
	rm -f $(DESTDIR)$(PREFIX)/lib/libcyn.so.$(SOVERSION)
	rm -f $(DESTDIR)$(PREFIX)/lib/libcyn.so
	rm -f $(DESTDIR)$(PREFIX)/lib/pkgconfig/libcyn.pc
	rm -f $(DESTDIR)$(PREFIX)/share/man/man3/cyn.3
	@echo "libcyn uninstalled from $(DESTDIR)$(PREFIX)"

clean:
	rm -f $(OBJ) libcyn.a libcyn.so libcyn.so.$(SOVERSION)
check: all
	$(CC) $(CFLAGS) tests/test_cyn.c -L. -lcyn -o test_cyn
	LD_LIBRARY_PATH=. ./test_cyn
	$(CC) $(CFLAGS) examples/demo.c -L. -lcyn -o demo
	LD_LIBRARY_PATH=. ./demo

check-strict: clean
	$(MAKE) all CFLAGS="$(CFLAGS) -Werror -pedantic"
	$(MAKE) check