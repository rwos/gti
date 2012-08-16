CC=gcc
CFLAGS=-O2 -Wall -Wextra

PROG=gti
SOURCES=gti.c animation.c

M4_DIR=m4
M4_GENERATED=animation.c animation.h

PREFIX=/usr/bin

ANIM_FRAMES=$(wildcard ascii/car2*.txt)
ANIM_HEIGHT=$(shell wc -l $(ANIM_FRAMES) | head -n-1 | awk '{ print $$1 }' \
						| sort -nr | head -n1)
ANIM_PPC=$(ANIM_FRAMES:%=%.ppc)

# define variables containing a space and a comma character, so we can
# replace the space characters by commas without confusing make too much
# (at the expense of possibly confusing the reader)
# also, don't use spaces in frame filenames for the time being ;-)
space :=
space +=
comma :=,
ANIM_PPC_M4=$(subst $(space),$(comma),$(ANIM_PPC))

all: $(PROG)

$(PROG): $(SOURCES)
	$(CC) -o $@ $(CFLAGS) $^
	#-strip -s $@

animation.%: $(M4_DIR)/animation_%.m4 $(ANIM_PPC)
	m4 -P -Dm4_height=$(ANIM_HEIGHT) -Dm4_frames='$(ANIM_PPC_M4)' $< > $@

# PreProCess ascii art files as to obtain a C friendly format
# in order, the substitution expressions (should) do the following:
# * escape '\'
# * escape '"'
# * add '"' to beginning of line, and indent to obtain a prettier source file
# * add '"," to end of line: mark end of string, and mark end of array element
$(ANIM_PPC): %.ppc: % Makefile
	 sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/^/    "/' -e 's/$$/",/' $< > $@

install: $(PROG)
	cp $^ $(PREFIX)

clean:
	rm -f $(PROG) $(ANIM_PPC) $(M4_GENERATED) make.depend

.PHONY: all install clean veryclean

make.depend: $(SOURCES) $(M4_GENERATED)
	$(CC) -MM $(SOURCES) > $@

     
ifneq ($(MAKECMDGOALS),clean)
include make.depend
endif
