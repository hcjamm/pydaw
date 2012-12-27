#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc-qt4

PREFIX ?= /usr

BASE_FLAGS     =  -mstackrealign -fsingle-precision-constant -fPIC -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -Wall -Isrc
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
BUILD_CXXFLAGS = $(BASE_FLAGS) $(shell pkg-config --cflags liblo QtCore QtGui x11 sm) $(CXXFLAGS)
LINK_CFLAGS    = -shared -lm $(LDFLAGS) $(shell pkg-config --libs liblo alsa)
LINK_CXXFLAGS  = -lm $(shell pkg-config --libs liblo QtCore QtGui x11 sm) $(LDFLAGS)

CXX_OBJS = src/synth_qt_gui.o src/moc_synth_qt_gui.o

# --------------------------------------------------------------

all: LMS_RAYV_qt

LMS_RAYV_qt: $(CXX_OBJS)
	$(CXX) $(CXX_OBJS) $(LINK_CXXFLAGS) -o $@

# --------------------------------------------------------------

.cpp.o:
	$(CXX) -c $< $(BUILD_CXXFLAGS) -o $@

src/moc_synth_qt_gui.cpp: src/synth_qt_gui.h
	$(MOC) $< -o $@

# --------------------------------------------------------------

strip:
	strip LMS_RAYV_qt

# --------------------------------------------------------------

install:
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw2
	install -d $(DESTDIR)$(PREFIX)/lib/pydaw2/ray_v
	install -m 755 LMS_RAYV_qt $(DESTDIR)$(PREFIX)/lib/pydaw2/ray_v

clean:
	rm -f src/*.o src/moc_*.cpp LMS_RAYV_qt

