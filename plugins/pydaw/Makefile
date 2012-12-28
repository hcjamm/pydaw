
#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc-qt4

PREFIX ?= /usr

BASE_FLAGS     = -ffast-math -fPIC -mstackrealign -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fprefetch-loop-arrays -floop-optimize -fsingle-precision-constant -Wall -Isrc -I.
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS) $(shell pkg-config --cflags alsa jack liblo) -I./dssi
LINK_CFLAGS    = -lm -ldl $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile jack)

C_OBJS   = src/jack-dssi-host.o src/message_buffer.o

# --------------------------------------------------------------

all: pydaw2

debug:
	make CFLAGS+=" -O0 -g -gdwarf-3 "

release:
	make CFLAGS+=" -O3 "

pydaw2: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

strip:
	strip pydaw2

# --------------------------------------------------------------

install:
	rm -rf python/*.pyc python/libpydaw/*.pyc python/*~ python/libpydaw/*~
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 pydaw2 $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw2
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw2/pydaw
	install -m 755 PYDAW_qt $(DESTDIR)$(PREFIX)/lib/pydaw2/pydaw
	cp -R python $(DESTDIR)$(PREFIX)/lib/pydaw2/pydaw

clean:
	rm -f src/*.o *.so python/*.pyc pydaw2
