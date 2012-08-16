CC=gcc
CFLAGS=-O2 -Wall -Wextra

PROG=gti
PREFIX=/usr/bin

$(PROG): *.c
	$(CC) -o $@ $(CFLAGS) $^
	-strip -s $@

install: $(PROG)
	cp $^ $(PREFIX)

.PHONY: clean
clean:
	rm -f $(PROG)

