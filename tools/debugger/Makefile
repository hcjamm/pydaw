#!/usr/bin/make -f

CC  ?= g++
CXX ?= g++
MOC ?= moc

PREFIX ?= /usr/local

BASE_FLAGS     = -O0 -gdwarf-3 -mstackrealign -fsingle-precision-constant -fPIC -mtune=generic -msse -msse2 -msse3 -mfpmath=sse
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
BUILD_CXXFLAGS = $(BASE_FLAGS) $(shell pkg-config --cflags liblo QtCore QtGui x11 sm sndfile) $(CXXFLAGS)
LINK_CFLAGS    = -ldl -shared -lm $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile)
LINK_CXXFLAGS  = -ldl -lm $(shell pkg-config --libs liblo QtCore QtGui x11 sm alsa sndfile) $(LDFLAGS)

C_OBJS   = main.o
CXX_OBJS = main.o

# --------------------------------------------------------------

all: debugger

debugger: $(CXX_OBJS)
	$(CXX) $(CXX_OBJS) $(LINK_CXXFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

.cpp.o:
	$(CXX) -c $< $(BUILD_CXXFLAGS) -o $@

# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/lib/dssi
	install -d $(DESTDIR)$(PREFIX)/lib/dssi/ray_v
	install -m 644 ray_v.so $(DESTDIR)$(PREFIX)/lib/dssi
	install -m 755 LMS_RAYV_qt $(DESTDIR)$(PREFIX)/lib/dssi/ray_v

clean:
	rm -f *.o *.so debugger

