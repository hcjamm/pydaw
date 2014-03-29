#!/usr/bin/make -f

PREFIX ?= /usr
DESTDIR ?= 
CC  ?= gcc
PYDAW_VERSION = $(shell cat ../major-version.txt)
PYDAW_FILE = $(PYDAW_VERSION)_render

BASE_FLAGS     = -Wall -mstackrealign -fsingle-precision-constant -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fstrength-reduce -fstrict-aliasing -fprefetch-loop-arrays -finline-functions
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
LINK_CFLAGS    = -ldl -lm $(shell pkg-config --libs liblo sndfile) $(LDFLAGS)

C_OBJS   = main.o

# --------------------------------------------------------------

all: $(PYDAW_FILE)

debug:
	make CFLAGS+=" -O0 -gdwarf-3 -Wstack-usage=4096 -fstack-usage "

release:
	make CFLAGS+=" -O2 "

gprof:
	make CFLAGS+=" -O2 -pg " LDFLAGS+=" -pg "

$(PYDAW_FILE): $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

clean:
	rm -f *.o *.so $(PYDAW_FILE) *.pymap *.wav profile.txt gmon.out

install:
	install -d $(DESTDIR)/$(PREFIX)
	install -d $(DESTDIR)/$(PREFIX)/bin
	install -m 755 $(PYDAW_FILE) $(DESTDIR)/$(PREFIX)/bin

