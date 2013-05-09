SHELL=/bin/sh

CC=gcc
CFLAGS+=-O2 -Wall -Wextra

INSTALL=install -D

BINDIR=$(DESTDIR)/usr/bin

PROG=gti

$(PROG): *.c
	$(CC) -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^
	-strip -s $@

install: $(PROG)
	$(INSTALL) $(PROG) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(PROG)

.PHONY: clean install uninstall
clean:
	rm -f $(PROG)
