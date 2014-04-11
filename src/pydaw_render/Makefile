#!/usr/bin/make -f

PREFIX ?= /usr
DESTDIR ?= 
CC  ?= gcc
PYDAW_VERSION = $(shell cat ../major-version.txt)
PYDAW_FILE = $(PYDAW_VERSION)_render
PYDAW_DBG = $(PYDAW_FILE)-dbg

#BASE_FLAGS     = -Wall -mstackrealign -fsingle-precision-constant -mtune=generic -msse 
#-msse2 -msse3 -mfpmath=sse -fstrength-reduce -fstrict-aliasing -fprefetch-loop-arrays -finline-functions

#Default flags are mostly x86 specific
PLAT_FLAGS     ?= -mstackrealign -msse -msse2 -msse3 -mfpmath=sse \
-mtune=generic

BASE_FLAGS     = -ffast-math -fprefetch-loop-arrays \
-floop-optimize -fsingle-precision-constant -funroll-loops -Wall \
-Isrc -Iportaudio/include -fstrength-reduce -fstrict-aliasing -finline-functions


BUILD_CFLAGS   = $(PLAT_FLAGS) $(BASE_FLAGS) $(CFLAGS)
LINK_CFLAGS    = -ldl -lm $(shell pkg-config --libs liblo sndfile) $(LDFLAGS)

C_OBJS   = main.o

# --------------------------------------------------------------

all:
	make debug
	make release

debug:
	rm *.o || true
	make CFLAGS+=" -O0 -gdwarf-3 " $(PYDAW_FILE)
	mv $(PYDAW_FILE) $(PYDAW_DBG)

release:
	rm *.o || true
	make CFLAGS+=" -O2 " $(PYDAW_FILE)

gprof:
	make CFLAGS+=" -O2 -pg " LDFLAGS+=" -pg "

$(PYDAW_FILE): $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

clean:
	rm -f *.o *.so $(PYDAW_FILE) $(PYDAW_DBG) *.pymap *.wav \
	profile.txt gmon.out

install:
	install -d $(DESTDIR)/$(PREFIX)
	install -d $(DESTDIR)/$(PREFIX)/bin
	install -s -m 755 $(PYDAW_FILE) $(DESTDIR)/$(PREFIX)/bin
	install -m 755 $(PYDAW_DBG) $(DESTDIR)/$(PREFIX)/bin

