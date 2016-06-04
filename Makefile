SHELL=/bin/sh

CC=cc
CFLAGS+=-O2 -Wall -Wextra

INSTALL=install -D
INSTALL_DATA=$(INSTALL) -m 644

BINDIR=$(DESTDIR)/usr/bin
MANDIR=$(DESTDIR)/usr/share/man/man6

STRIP=strip
ifeq ($(OS),Windows_NT)
 X = .exe
else
  ifeq ($(shell uname), SunOS)
    STRIP=gstrip
  endif
endif

PROG=gti$X
MANPAGE=gti.6.gz

$(PROG): *.c
	$(CC) -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^
	-$(STRIP) -s $@

$(MANPAGE): gti.6
	gzip -9 -n -c gti.6 > gti.6.gz

install: $(PROG) $(MANPAGE)
	$(INSTALL) $(PROG) $(BINDIR)/$(PROG)
	$(INSTALL_DATA) $(MANPAGE) $(MANDIR)/$(MANPAGE)

uninstall:
	rm -f $(BINDIR)/$(PROG)
	rm -f $(MANDIR)/$(MANPAGE)

.PHONY: clean install uninstall
clean:
	rm -f $(PROG)
	rm -f $(MANPAGE)
