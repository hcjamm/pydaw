#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc

PREFIX ?= /usr/local

BASE_FLAGS     = -ffast-math -fPIC -mstackrealign -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fprefetch-loop-arrays -floop-optimize -fsingle-precision-constant -Wall -Isrc -I.
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
BUILD_CXXFLAGS = $(BASE_FLAGS) $(shell pkg-config --cflags liblo QtCore QtGui x11 sm sndfile) $(CXXFLAGS)
LINK_CFLAGS    = -shared -lm $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile)
LINK_CXXFLAGS  = -lm -pthread $(shell pkg-config --libs liblo QtCore QtGui x11 sm sndfile) $(LDFLAGS)

C_OBJS   = src/synth.o
CXX_OBJS = src/synth_qt_gui.o src/moc_synth_qt_gui.o

# --------------------------------------------------------------

all: euphoria.so LMS_EUPHORIA_qt

euphoria.so: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS)  -o $@

LMS_EUPHORIA_qt: $(CXX_OBJS)
	$(CXX) $(CXX_OBJS) $(LINK_CXXFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

.cpp.o:
	$(CXX) -c $< $(BUILD_CXXFLAGS) -o $@

src/moc_synth_qt_gui.cpp: src/synth_qt_gui.h
	$(MOC) $< -o $@

# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/lib/dssi
	install -d $(DESTDIR)$(PREFIX)/lib/dssi/euphoria
	install -m 644 euphoria.so $(DESTDIR)$(PREFIX)/lib/dssi
	install -m 755 LMS_EUPHORIA_qt $(DESTDIR)$(PREFIX)/lib/dssi/euphoria

clean:
	rm -f src/*.o src/moc_*.cpp *.so LMS_EUPHORIA_qt

