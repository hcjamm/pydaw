
#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc-qt4

PREFIX ?= /usr

BASE_FLAGS     = -ffast-math -mstackrealign -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fprefetch-loop-arrays -floop-optimize -fsingle-precision-constant -funroll-loops -Wall -Isrc -I. -fstrength-reduce -fstrict-aliasing -finline-functions
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS) $(shell pkg-config --cflags alsa jack liblo)
LINK_CFLAGS    = -lm -ldl $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile jack)

C_OBJS   = src/main.o src/message_buffer.o

# --------------------------------------------------------------

all: pydaw3

debug:
	make CFLAGS+=" -O0 -g -gdwarf-3 "

release:
	make CFLAGS+=" -O3 "

pydaw3: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

strip:
	strip pydaw3

# --------------------------------------------------------------

install:
	rm -rf python/*.pyc python/libpydaw/*.pyc python/*~ python/libpydaw/*~
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 pydaw3 $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw3
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw3/pydaw
	install -m 755 PYDAW_qt $(DESTDIR)$(PREFIX)/lib/pydaw3/pydaw
	cp -R python $(DESTDIR)$(PREFIX)/lib/pydaw3/pydaw

clean:
	rm -f src/*.o *.so python/*.pyc python/libpydaw/*.pyc python/pydaw/*~ pydaw3 core *~ src/*~
