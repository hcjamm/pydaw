#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++

PREFIX ?= /usr
PYDAW_VERSION = pydaw3

#Default flags are mostly x86 specific
PLAT_FLAGS     ?= -mstackrealign -msse -msse2 -msse3 -mfpmath=sse

BASE_FLAGS     = -ffast-math -mtune=generic -fprefetch-loop-arrays \
-floop-optimize -fsingle-precision-constant -funroll-loops -Wall \
-Isrc -Iportaudio/include -fstrength-reduce -fstrict-aliasing -finline-functions

PYDAW_LIB_DIR   ?= /usr/lib
PYDAW_LIB64_DIR ?= $(shell if [ -e "/usr/lib/x86_64-linux-gnu" ]; \
then printf "/usr/lib/x86_64-linux-gnu" ; \
elif [ -e "/usr/lib64" ]; \
then printf "/usr/lib64" ; \
fi)

BUILD_CFLAGS   = $(PLAT_FLAGS) $(BASE_FLAGS) $(CFLAGS)

PORTAUDIO_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/libportaudio.a ]; \
then printf "$(PYDAW_LIB_DIR)/libportaudio.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/libportaudio.a ]; \
then printf "$(PYDAW_LIB64_DIR)/libportaudio.a" ; \
else printf " -lportaudio" ; \
fi)

PORTMIDI_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/libportmidi.a ]; \
then printf "$(PYDAW_LIB_DIR)/libportmidi.a $(PYDAW_LIB_DIR)/libporttime.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/libportmidi.a ]; \
then printf "$(PYDAW_LIB64_DIR)/libportmidi.a $(PYDAW_LIB64_DIR)/libporttime.a" ; \
else printf " -lportmidi" ; \
fi)

LIBLO_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/liblo.a ]; \
then printf "$(PYDAW_LIB_DIR)/liblo.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/liblo.a ]; \
then printf "$(PYDAW_LIB64_DIR)/liblo.a" ; \
else printf " -llo" ; \
fi)

MATH_LIB_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/libm.a ]; \
then printf "$(PYDAW_LIB_DIR)/libm.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/libm.a ]; \
then printf "$(PYDAW_LIB64_DIR)/libm.a" ; \
else printf " -lm" ; \
fi)

RT_LIB_FLAG ?= $(shell if [ -e $(PYDAW_LIB_DIR)/librt.a ]; \
then printf "$(PYDAW_LIB_DIR)/librt.a" ; \
elif [ -e $(PYDAW_LIB64_DIR)/librt.a ]; \
then printf "$(PYDAW_LIB64_DIR)/librt.a" ; \
else printf " -lrt" ; \
fi)

LINK_CFLAGS    = $(RT_LIB_FLAG) -lpthread $(LIBLO_FLAG) $(PORTMIDI_FLAG) \
portaudio/lib/.libs/libportaudio.a \
$(LDFLAGS) $(shell pkg-config --libs sndfile alsa) \
$(MATH_LIB_FLAG)

C_OBJS   = src/main.o

# --------------------------------------------------------------

all:
	make -C midicomp && cp midicomp/midicomp python/libpydaw
	cd pyliblo-0.9.1 && python3 setup.py build && cp build/lib*/*.so ../python/libpydaw
	cd portaudio && ./configure --with-jack=no --with-oss=no
	make -C portaudio
	cp portaudio/lib/.libs/libportaudio.so.2.0.0 python/libpydaw/libportaudio.so
	make $(PYDAW_VERSION)-engine

debug:
	make CFLAGS+=" -O0 -g -gdwarf-3 "

release:
	make CFLAGS+=" -O2 "

$(PYDAW_VERSION)-engine: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

strip:
	strip $(PYDAW_VERSION)-engine

# --------------------------------------------------------------

install:
	rm -rf python/*.pyc python/libpydaw/*.pyc python/*~ python/libpydaw/*~
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(PYDAW_VERSION) $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/pydaw
	install -m 755 $(PYDAW_VERSION)-project-recover $(DESTDIR)$(PREFIX)/bin
	install -m 4755 $(PYDAW_VERSION)-engine $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(PYDAW_VERSION)-engine $(DESTDIR)$(PREFIX)/bin/$(PYDAW_VERSION)-engine-no-root
	cp -r python $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/pydaw
	cp -r cc_maps $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r themes $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r presets $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r share $(DESTDIR)$(PREFIX)
	install -m 644 ../$(PYDAW_VERSION)-version.txt $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	#Add the Git revision to the version.txt file if possible
	@       if [ -e /usr/bin/git ] && [ -e ../.git ]; then \
                        echo "\n\nGit: " `/usr/bin/git rev-parse HEAD` >> $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/$(PYDAW_VERSION)-version.txt ; \
                fi

clean:
	rm -f src/*.o *.so python/*.pyc python/libpydaw/*.pyc python/pydaw/*~ $(PYDAW_VERSION)-engine core python/core *~ src/*~

