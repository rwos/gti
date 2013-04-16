SHELL=/bin/sh

CC=gcc
CFLAGS=-O2 -Wall -Wextra

PROG=gti
PREFIX=$(DESTDIR)/usr/bin

$(PROG): *.c
	$(CC) -o $@ $(CFLAGS) $^
	-strip -s $@

install: $(PROG)
	cp $^ $(PREFIX)

uninstall:
	rm -f $(PREFIX)/$(PROG)

.PHONY: clean install uninstall
clean:
	rm -f $(PROG)

