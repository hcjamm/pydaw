
#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc-qt4

PREFIX ?= /usr

BASE_FLAGS     = -ffast-math -floop-optimize -fPIC -msse -msse2 -msse3 -mfpmath=sse -Wall -Isrc -I.
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
LINK_CFLAGS    = -shared -lm -ldl $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile)

C_OBJS   = src/synth.o

# --------------------------------------------------------------

all: pydaw.so

pydaw.so: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib/dssi
	install -d $(DESTDIR)$(PREFIX)/lib/dssi/pydaw
	install -m 644 pydaw.so $(DESTDIR)$(PREFIX)/lib/dssi
	install -m 755 PYDAW_qt $(DESTDIR)$(PREFIX)/lib/dssi/pydaw
	install -m 755 pydaw $(DESTDIR)$(PREFIX)/bin
	cp -R python $(DESTDIR)$(PREFIX)/lib/dssi/pydaw

clean:
	rm -f src/*.o *.so python/*.pyc
