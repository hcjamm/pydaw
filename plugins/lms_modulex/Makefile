#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc-qt4

PREFIX ?= /usr

BASE_FLAGS     = -mstackrealign -ffast-math -fPIC -mtune=generic -mfpmath=sse -msse -msse2 -msse3 -fsingle-precision-constant -Wall -Isrc -I.
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
BUILD_CXXFLAGS = $(BASE_FLAGS) $(shell pkg-config --cflags liblo QtCore QtGui x11 sm sndfile) $(CXXFLAGS)
LINK_CFLAGS    = -shared -lm $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile samplerate)
LINK_CXXFLAGS  = -lm -pthread $(shell pkg-config --libs liblo QtCore QtGui x11 sm sndfile) $(LDFLAGS)

C_OBJS   = src/synth.o
CXX_OBJS = src/synth_qt_gui.o src/moc_synth_qt_gui.o

# --------------------------------------------------------------

all: LMS_MODULEX_qt

LMS_MODULEX_qt: $(CXX_OBJS)
	$(CXX) $(CXX_OBJS) $(LINK_CXXFLAGS) -o $@

# --------------------------------------------------------------

.cpp.o:
	$(CXX) -c $< $(BUILD_CXXFLAGS) -o $@

src/moc_synth_qt_gui.cpp: src/synth_qt_gui.h
	$(MOC) $< -o $@

# --------------------------------------------------------------

strip:
	strip LMS_MODULEX_qt

# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw2
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw2/lms_modulex
	install -m 755 LMS_MODULEX_qt $(DESTDIR)$(PREFIX)/lib/pydaw2/lms_modulex

clean:
	rm -f src/*.o src/moc_*.cpp LMS_MODULEX_qt

