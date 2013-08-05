#!/usr/bin/make -f

#Prefix installs aren't currently supported because paths have been hard-coded for Qt stylesheets
#and a couple of other scenarios where it's very inconvenient to have to allow the path to change...
PREFIX = /usr
DESTDIR ?=
SBSMS_PREFIX = $(PREFIX)/lib/$(PYDAW_VERSION)/sbsms
RUBBERBAND_PREFIX = $(PREFIX)/lib/$(PYDAW_VERSION)/rubberband
PYDAW_VERSION = pydaw3

# --------------------------------------------------------------

all:
	make -C pydaw CFLAGS+=" -O3 "
	make -C euphoria CFLAGS+=" -O3 "
	make -C ray_v CFLAGS+=" -O3 "
	#make -C lms_modulex CFLAGS+=" -O3 "
	make -C way_v CFLAGS+=" -O3 "
	make -C pydaw_jack_oscrolloscope
	cd pydaw_sbsms && ./configure --prefix=$(SBSMS_PREFIX) && make && cd ..
	cd pydaw_sbsms/cli && ./configure --prefix=$(SBSMS_PREFIX) && make LDFLAGS+="-pthread" && cd ../..
	cd pydaw_rubberband && ./configure --prefix=$(RUBBERBAND_PREFIX) && make && cd ..
		
# --------------------------------------------------------------
debug:
	make -C pydaw CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C euphoria CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C ray_v CFLAGS+=" -O0 -g -gdwarf-3 "
	#make -C lms_modulex CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C way_v CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C pydaw_jack_oscrolloscope
        cd pydaw_sbsms && make distclean && ./configure --prefix=$(SBSMS_PREFIX) && make && cd ..
        cd pydaw_sbsms/cli && make distclean && ./configure --prefix=$(SBSMS_PREFIX) && make LDFLAGS+="-pthread" && cd ..
	cd pydaw_rubberband && make distclean && ./configure --prefix=$(RUBBERBAND_PREFIX) && make && cd ..

# --------------------------------------------------------------

strip:
	make -C pydaw strip
	make -C euphoria strip
	make -C ray_v strip
	#make -C lms_modulex strip
	make -C way_v strip
	make -C pydaw_jack_oscrolloscope strip

# --------------------------------------------------------------
install:
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/sbsms
	install -d $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)/rubberband
	make -C pydaw PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C euphoria PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C ray_v PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	#make -C lms_modulex PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C way_v PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C pydaw_jack_oscrolloscope PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	cp -r share $(DESTDIR)$(PREFIX)
	cp -r themes $(DESTDIR)$(PREFIX)/lib/$(PYDAW_VERSION)
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
	make -C euphoria clean
	make -C ray_v clean
	#make -C lms_modulex clean
	make -C way_v clean
	make -C pydaw_jack_oscrolloscope clean
	#make -C pydaw_sbsms clean
	#make -C pydaw_sbsms/cli clean
	#make -C pydaw_rubberband clean

