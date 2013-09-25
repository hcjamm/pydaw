#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++

PREFIX ?= /usr
PYDAW_VERSION = pydaw3

BASE_FLAGS     = -ffast-math -mstackrealign -mtune=generic -msse -msse2 -msse3 -mfpmath=sse \
-fprefetch-loop-arrays -floop-optimize -fsingle-precision-constant -funroll-loops -Wall \
-Isrc -I. -fstrength-reduce -fstrict-aliasing -finline-functions

PYDAW_LIB_DIR   ?= /usr/lib
PYDAW_LIB64_DIR ?= /usr/lib/x86_64-linux-gnu

BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS) $(shell pkg-config --cflags liblo)

PORTAUDIO_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/libportaudio.a ]; \
then printf "$(PYDAW_LIB_DIR)/libportaudio.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/libportaudio.a ]; \
then printf "$(PYDAW_LIB64_DIR)/libportaudio.a" ; \
else printf "-lportaudio" ; \
fi)

PORTMIDI_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/libportmidi.a ]; \
then printf "$(PYDAW_LIB_DIR)/libportmidi.a $(PYDAW_LIB_DIR)/libporttime.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/libportmidi.a ]; \
then printf "$(PYDAW_LIB64_DIR)/libportmidi.a $(PYDAW_LIB64_DIR)/libporttime.a" ; \
else printf "-lportmidi" ; \
fi)

LIBLO_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/liblo.a ]; \
then printf "$(PYDAW_LIB_DIR)/liblo.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/liblo.a ]; \
then printf "$(PYDAW_LIB64_DIR)/liblo.a" ; \
else printf "-llo" ; \
fi)

MATH_LIB_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/libm.a ]; \
then printf "$(PYDAW_LIB_DIR)/libm.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/libm.a ]; \
then printf "$(PYDAW_LIB64_DIR)/libm.a" ; \
else printf "-lm" ; \
fi)

LINK_CFLAGS    = -lpthread $(MATH_LIB_FLAG) $(LIBLO_FLAG) $(PORTMIDI_FLAG) \
portaudio/lib/.libs/libportaudio.a \
$(LDFLAGS) $(shell pkg-config --libs sndfile alsa)

C_OBJS   = src/main.o

# --------------------------------------------------------------

all:
	cd portaudio && ./configure --with-jack=no --with-oss=no
	make -C portaudio
	make pydaw3-engine

debug:
	make CFLAGS+=" -O0 -g -gdwarf-3 "

release:
	make CFLAGS+=" -O2 "

pydaw3-engine: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

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
