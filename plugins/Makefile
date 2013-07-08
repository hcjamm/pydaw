#!/usr/bin/make -f

#Prefix installs aren't currently supported because paths have been hard-coded for Qt stylesheets
#and a couple of other scenarios where it's very inconvenient to have to allow the path to change...
PREFIX = /usr
DESTDIR ?=

# --------------------------------------------------------------

all:
	make -C pydaw CFLAGS+=" -O3 "
	make -C euphoria CFLAGS+=" -O3 "
	make -C ray_v CFLAGS+=" -O3 "
	make -C lms_modulex CFLAGS+=" -O3 "
	make -C way_v CFLAGS+=" -O3 "
	make -C pydaw_jack_oscrolloscope
	
# --------------------------------------------------------------
debug:
	make -C pydaw CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C euphoria CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C ray_v CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C lms_modulex CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C way_v CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C pydaw_jack_oscrolloscope

# --------------------------------------------------------------

strip:
	make -C pydaw strip
	make -C euphoria strip
	make -C ray_v strip
	make -C lms_modulex strip
	make -C way_v strip
	make -C pydaw_jack_oscrolloscope strip

# --------------------------------------------------------------
install:
	make -C pydaw PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C euphoria PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C ray_v PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C lms_modulex PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C way_v PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C pydaw_jack_oscrolloscope PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw3
	cp -r share $(DESTDIR)$(PREFIX)
	cp -r themes $(DESTDIR)$(PREFIX)/lib/pydaw3
	cp -r cc_maps $(DESTDIR)$(PREFIX)/lib/pydaw3
	cp -r mixxx $(DESTDIR)$(PREFIX)/lib/pydaw3
	install -m 755 pydaw3-version.txt $(DESTDIR)$(PREFIX)/lib/pydaw3
	#Add the Git revision to the version.txt file if possible
	@	if [ -e /usr/bin/git ] && [ -e ../.git ]; then \
			echo "\n\nGit: " `/usr/bin/git rev-parse HEAD` >> $(DESTDIR)$(PREFIX)/lib/pydaw3/pydaw3-version.txt ; \
		fi

clean:
	make -C pydaw clean
	make -C euphoria clean
	make -C ray_v clean
	make -C lms_modulex clean
	make -C way_v clean
	make -C pydaw_jack_oscrolloscope clean

