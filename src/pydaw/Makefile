
#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++

PREFIX ?= /usr
PYDAW_VERSION = pydaw3

BASE_FLAGS     = -ffast-math -mstackrealign -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fprefetch-loop-arrays -floop-optimize -fsingle-precision-constant -funroll-loops -Wall -Isrc -I. -fstrength-reduce -fstrict-aliasing -finline-functions
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS) $(shell pkg-config --cflags alsa liblo)
LINK_CFLAGS    = -lm -ldl -lportaudio $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile)

C_OBJS   = src/main.o

# --------------------------------------------------------------

all: pydaw3-engine

debug:
	make CFLAGS+=" -O0 -g -gdwarf-3 "

release:
	make CFLAGS+=" -O3 "

pydaw3-engine: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -DPYDAW_PREFIX='"$(PREFIX)"' -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

strip:
	strip pydaw3-engine

# --------------------------------------------------------------

install:
	rm -rf python/*.pyc python/libpydaw/*.pyc python/*~ python/libpydaw/*~
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 pydaw3 $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw3
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw3/pydaw
	install -m 755 pydaw3-project-recover $(DESTDIR)$(PREFIX)/bin
	install -m 755 pydaw3-engine $(DESTDIR)$(PREFIX)/bin
	cp -r python $(DESTDIR)$(PREFIX)/lib/pydaw3/pydaw
	cp -r cc_maps $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r themes $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r presets $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r share $(DESTDIR)$(PREFIX)
	install -m 755 ../$(PYDAW_VERSION)-version.txt $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	#Add the Git revision to the version.txt file if possible
	@       if [ -e /usr/bin/git ] && [ -e ../.git ]; then \
                        echo "\n\nGit: " `/usr/bin/git rev-parse HEAD` >> $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/$(PYDAW_VERSION)-version.txt ; \
                fi

clean:
	rm -f src/*.o *.so python/*.pyc python/libpydaw/*.pyc python/pydaw/*~ pydaw3-engine core python/core *~ src/*~

