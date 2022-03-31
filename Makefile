SHELL=/bin/sh

# DESTDIR is used in a non-standard way for compatibility
prefix=$(DESTDIR)/usr

CC=cc
CFLAGS+=-O2 -std=c89 -Wpedantic -Wall -Wextra -Wunused -Wshadow -Wdouble-promotion -Wstrict-overflow=5

INSTALL=install
INSTALL_DATA=$(INSTALL) -m 644

bindir=$(prefix)/bin
datarootdir=$(prefix)/share
mandir=$(datarootdir)/man
man6dir=$(mandir)/man6
BASHCOMPLDIR=$(datarootdir)/bash-completion/completions
ZSHCOMPLDIR=$(datarootdir)/zsh/site-functions

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
	$(INSTALL) $(PROG) $(bindir)/$(PROG)
	$(INSTALL_DATA) $(MANPAGE) $(man6dir)/$(MANPAGE)
	$(INSTALL_DATA) completions/gti.bash $(BASHCOMPLDIR)/$(PROG)
	$(INSTALL_DATA) completions/gti.zsh $(ZSHCOMPLDIR)/_$(PROG)

uninstall:
	rm -f $(bindir)/$(PROG)
	rm -f $(man6dir)/$(MANPAGE)

fmt: *.c
	VERSION_CONTROL=never indent -kr -i4 -ppi4 -nut -l100 -cp0 -ncs -sob \
		-T HANDLE \
		$^

.PHONY: clean install uninstall
clean:
	rm -f $(PROG)
	rm -f $(MANPAGE)
