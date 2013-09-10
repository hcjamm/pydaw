#!/usr/bin/make -f

PREFIX ?= /usr
DESTDIR ?=
SBSMS_PREFIX = $(PREFIX)/lib/$(PYDAW_VERSION)/sbsms
RUBBERBAND_PREFIX = $(PREFIX)/lib/$(PYDAW_VERSION)/rubberband
PYDAW_VERSION = pydaw3

# --------------------------------------------------------------

all:
	make -C pydaw PREFIX=$(PREFIX) CFLAGS+=" -O3 "
	#make -C pydaw_jack_oscrolloscope
	cd pydaw_sbsms && ./configure --prefix=$(SBSMS_PREFIX) && make && cd ..
	cd pydaw_sbsms/cli && ./configure --prefix=$(SBSMS_PREFIX) && make LDFLAGS+="-pthread" && cd ../..
	cd pydaw_rubberband && ./configure --prefix=$(RUBBERBAND_PREFIX) && make && cd ..
		
# --------------------------------------------------------------
debug:
	make -C pydaw PREFIX=$(PREFIX) CFLAGS+=" -O0 -g -gdwarf-3 "
	#make -C pydaw_jack_oscrolloscope
	cd pydaw_sbsms && make distclean && ./configure --prefix=$(SBSMS_PREFIX) && make && cd ..
	cd pydaw_sbsms/cli && make distclean && ./configure --prefix=$(SBSMS_PREFIX) && make LDFLAGS+="-pthread" && cd ..
	cd pydaw_rubberband && make distclean && ./configure --prefix=$(RUBBERBAND_PREFIX) && make && cd ..

# --------------------------------------------------------------

strip:
	make -C pydaw strip
	#make -C pydaw_jack_oscrolloscope strip

# --------------------------------------------------------------
install:
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/sbsms
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/rubberband
	make -C pydaw PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	#make -C pydaw_jack_oscrolloscope PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	cp -r share $(DESTDIR)$(PREFIX)
	cp -r themes $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r presets $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r cc_maps $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	cp -r mixxx $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	install -m 755 $(PYDAW_VERSION)-version.txt $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	make -C pydaw_sbsms DESTDIR=$(DESTDIR) install
	make -C pydaw_sbsms/cli DESTDIR=$(DESTDIR) install
	make -C pydaw_rubberband DESTDIR=$(DESTDIR) install
	#The configure script doesn't offer a lot of granularity for what to install, so just snip out the parts we don't want
	cd $(DESTDIR)/$(RUBBERBAND_PREFIX) ; rm -rf lib/ladspa lib/vamp include share
	#Add the Git revision to the version.txt file if possible
	@	if [ -e /usr/bin/git ] && [ -e ../.git ]; then \
			echo "\n\nGit: " `/usr/bin/git rev-parse HEAD` >> $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/$(PYDAW_VERSION)-version.txt ; \
		fi

clean:
	make -C pydaw clean
	#make -C pydaw_jack_oscrolloscope clean
	make -C pydaw_sbsms clean
	make -C pydaw_sbsms/cli clean
	make -C pydaw_rubberband clean

