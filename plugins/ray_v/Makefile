#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc

PREFIX ?= /usr/local

BASE_FLAGS     = -O2 -ffast-math -fomit-frame-pointer -fvisibility=hidden -fPIC -mtune=generic -msse -Wall -Isrc
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
BUILD_CXXFLAGS = $(BASE_FLAGS) $(shell pkg-config --cflags liblo QtCore QtGui) $(CXXFLAGS)
LINK_CFLAGS    = -shared -lm $(LDFLAGS)
LINK_CXXFLAGS  = -lm $(shell pkg-config --libs liblo QtCore QtGui) $(LDFLAGS)

C_OBJS   = src/synth.o
CXX_OBJS = src/synth_qt_gui.o src/moc_synth_qt_gui.o

# --------------------------------------------------------------

all: ray_v.so LMS_RAYV_qt

ray_v.so: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

LMS_RAYV_qt: $(CXX_OBJS)
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
	install -d $(DESTDIR)$(PREFIX)/lib/dssi/ray_v
	install -m 644 ray_v.so $(DESTDIR)$(PREFIX)/lib/dssi
	install -m 755 LMS_RAYV_qt $(DESTDIR)$(PREFIX)/lib/dssi/ray_v

clean:
	rm -f src/*.o src/moc_*.cpp *.so LMS_RAYV_qt

