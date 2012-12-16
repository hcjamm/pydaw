#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++

PREFIX ?= /usr

BASE_FLAGS     = -ffast-math -fPIC -mstackrealign -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fprefetch-loop-arrays -floop-optimize -fsingle-precision-constant -Wall -Isrc -I.

BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS) $(shell pkg-config --cflags alsa jack liblo) -I./dssi

LINK_CFLAGS    = -lm -ldl $(LDFLAGS) $(shell pkg-config --libs liblo alsa jack)


C_OBJS   = jack-dssi-host.o message_buffer.o


# --------------------------------------------------------------

all: lms-jack-dssi-host

lms-jack-dssi-host: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS)  -o $@


# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 644 lms-jack-dssi-host $(DESTDIR)$(PREFIX)/bin

clean:
	rm -f *.o lms-jack-dssi-host

