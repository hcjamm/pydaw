#!/usr/bin/make -f

PREFIX ?= /usr
DESTDIR ?=

# --------------------------------------------------------------

all:
	make -C pydaw CFLAGS+=" -O3 "
	make -C lms-jack-dssi-host CFLAGS+=" -O3 "
	make -C euphoria CFLAGS+=" -O3 "
	make -C ray_v CFLAGS+=" -O3 "
	make -C lms_modulex CFLAGS+=" -O3 "
	make -C way_v CFLAGS+=" -O3 "
	make -C pydaw_jack_oscrolloscope
	
# --------------------------------------------------------------
debug:
	make -C pydaw CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C lms-jack-dssi-host CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C euphoria CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C ray_v CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C lms_modulex CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C way_v CFLAGS+=" -O0 -g -gdwarf-3 "
	make -C pydaw_jack_oscrolloscope CFLAGS+=" -O0 -g -gdwarf-3 "

# --------------------------------------------------------------

install:
	make -C pydaw PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C lms-jack-dssi-host PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C euphoria PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C ray_v PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C lms_modulex PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C way_v PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	make -C pydaw_jack_oscrolloscope PREFIX=$(PREFIX) DESTDIR=$(DESTDIR) install
	cp -r usr/ $(DESTDIR)$(PREFIX)

clean:
	make -C pydaw clean
	make -C lms-jack-dssi-host clean
	make -C euphoria clean
	make -C ray_v clean
	make -C lms_modulex clean
	make -C way_v clean
	make -C pydaw_jack_oscrolloscope clean

