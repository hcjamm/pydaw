#!/usr/bin/make -f

PREFIX ?= /usr
DESTDIR ?=
SBSMS_PREFIX = $(PREFIX)/lib/$(PYDAW_VERSION)/sbsms
RUBBERBAND_PREFIX = $(PREFIX)/lib/$(PYDAW_VERSION)/rubberband
PYDAW_VERSION = pydaw3

# --------------------------------------------------------------

all:
	make -C pydaw PREFIX=$(PREFIX) CFLAGS+=" -O2 "
	cd pydaw_sbsms && ./configure && make
	cd pydaw_sbsms/cli && make
	cd pydaw_rubberband && ./configure --prefix=$(RUBBERBAND_PREFIX) && make
		
# --------------------------------------------------------------

debug:
	make -C pydaw PREFIX=$(PREFIX) CFLAGS+=" -O0 -g -gdwarf-3 "
	cd pydaw_sbsms && make distclean && ./configure --prefix=$(SBSMS_PREFIX) && make
	make -C pydaw_sbsms/cli
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

clean:
	make -C pydaw clean
	#make -C pydaw_sbsms clean
	#make -C pydaw_sbsms/cli clean
	#make -C pydaw_rubberband clean

