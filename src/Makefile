#!/usr/bin/make -f

PREFIX ?= /usr
DESTDIR ?=
SBSMS_PREFIX = $(PREFIX)/lib/$(PYDAW_VERSION)/sbsms
RUBBERBAND_PREFIX = $(PREFIX)/lib/$(PYDAW_VERSION)/rubberband
PYDAW_VERSION = pydaw4

# --------------------------------------------------------------

all:
	make deps
	make pydaw

pydaw_src:
	make -C pydaw PREFIX=$(PREFIX) CFLAGS+=" -O2 "

# --------------------------------------------------------------

deps:
	make -C pydaw/midicomp && cp pydaw/midicomp/midicomp pydaw/python/libpydaw
	cd pydaw/pyliblo-0.9.1 && python3 setup.py build && cp build/lib*/*.so ../python/libpydaw
	cd pydaw/portaudio && ./configure --with-jack=no --with-oss=no
	make -C pydaw/portaudio
	cp pydaw/portaudio/lib/.libs/libportaudio.so.2.0.0 pydaw/python/libpydaw/libportaudio.so
	cd pydaw_sbsms && ./configure && make
	cd pydaw_sbsms/cli && make
	cd pydaw_rubberband && ./configure --prefix=$(RUBBERBAND_PREFIX) && make
		
# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/sbsms
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/rubberband
	make -C pydaw PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	cp -r mixxx $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	make -C pydaw_sbsms/cli PREFIX=$(SBSMS_PREFIX)  DESTDIR=$(DESTDIR) install
	make -C pydaw_rubberband DESTDIR=$(DESTDIR) install

# --------------------------------------------------------------

clean:
	make -C pydaw clean
	#make -C pydaw_sbsms clean
	#make -C pydaw_sbsms/cli clean
	#make -C pydaw_rubberband clean

