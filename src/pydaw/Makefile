#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++

PREFIX ?= /usr
PYDAW_VERSION = $(shell cat ../major-version.txt)

#Default flags are mostly x86 specific
PLAT_FLAGS     ?= -mstackrealign -msse -msse2 -msse3 -mfpmath=sse \
-mtune=generic

BASE_FLAGS     = -ffast-math -fprefetch-loop-arrays \
-floop-optimize -fsingle-precision-constant -funroll-loops -Wall \
-Isrc -Iportaudio/include -fstrength-reduce -fstrict-aliasing \
-finline-functions

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

LINK_CFLAGS  = $(RT_LIB_FLAG) -pthread $(LIBLO_FLAG) $(PORTMIDI_FLAG) \
portaudio/lib/.libs/libportaudio.a \
$(LDFLAGS) $(shell pkg-config --libs sndfile alsa fftw3) $(MATH_LIB_FLAG)

C_OBJS   = src/main.o

# --------------------------------------------------------------

all:
	make dbg
	make nohw
	make eng

dbg:
	rm src/*.o $(PYDAW_VERSION)-engine  $(PYDAW_VERSION)-engine-dbg ; echo
	make CFLAGS+=" -O0 -g -gdwarf-3 " $(PYDAW_VERSION)-engine
	mv $(PYDAW_VERSION)-engine $(PYDAW_VERSION)-engine-dbg

nohw:
	rm -f $(PYDAW_VERSION)-engine $(PYDAW_VERSION)-engine-no-hw src/*.o ; echo
	make CFLAGS+=" -O0 -g -gdwarf-3 -DPYDAW_NO_HARDWARE " $(PYDAW_VERSION)-engine
	mv $(PYDAW_VERSION)-engine $(PYDAW_VERSION)-engine-no-hw

eng:
	rm src/*.o $(PYDAW_VERSION)-engine ; echo
	rm -f $(PYDAW_VERSION)-engine src/*.o
	make CFLAGS+=" -O2 " $(PYDAW_VERSION)-engine

# --------------------------------------------------------------

$(PYDAW_VERSION)-engine: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

install:
	rm -rf python/*.pyc python/libpydaw/*.pyc python/*~ \
	python/libpydaw/*~ python/core
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(PYDAW_VERSION) $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/pydaw
	install -m 755 $(PYDAW_VERSION)-project-recover $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(PYDAW_VERSION)-engine-dbg $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(PYDAW_VERSION)-engine-no-hw $(DESTDIR)$(PREFIX)/bin
	install -s -m 4755 $(PYDAW_VERSION)-engine $(DESTDIR)$(PREFIX)/bin
	install -s -m 755 $(PYDAW_VERSION)-engine \
	$(DESTDIR)$(PREFIX)/bin/$(PYDAW_VERSION)-engine-no-root
	cp -r python $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/pydaw
	cp -r themes $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r presets $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r share $(DESTDIR)$(PREFIX)
	#locale/Makefile.py $(DESTDIR)$(PREFIX)
	install -m 644 ../major-version.txt \
		$(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	install -m 644 ../minor-version.txt \
		$(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	#Add the Git revision to the version.txt file if possible
	@       if [ -e /usr/bin/git ] && [ -e ../../.git ]; then \
                        echo "\n\nGit: " `/usr/bin/git rev-parse HEAD` >> \
			$(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/minor-version.txt ; \
                fi

clean:
	rm -f src/*.o *.so $(PYDAW_VERSION)-engine* core python/core  \
	*.pyo *.pyc */*.pyo */*.pyc */*/*.pyo */*/*.pyc \
	*~ */*~ */*/*~ */*/*/*~ 

