#!/usr/bin/make -f

CXX ?= g++
MOC ?= moc

PREFIX ?= /usr/local

BASE_FLAGS     = -O0 -gdwarf-3 -ffast-math -fPIC -mstackrealign -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fsingle-precision-constant -Wall -I.
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
BUILD_CXXFLAGS = $(BASE_FLAGS) $(shell pkg-config --cflags liblo QtCore QtGui x11 sm sndfile) $(CXXFLAGS)
LINK_CFLAGS    = -shared -lm $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile)
LINK_CXXFLAGS  = -lm -pthread $(shell pkg-config --libs liblo QtCore QtGui x11 sm sndfile) $(LDFLAGS)

CXX_OBJS = main.o moc_main_form.o

# --------------------------------------------------------------

all: lms_session_mgr

lms_session_mgr: $(CXX_OBJS)
	$(CXX) $(CXX_OBJS) $(LINK_CXXFLAGS) -o $@

# --------------------------------------------------------------

.cpp.o:
	$(CXX) -c $< $(BUILD_CXXFLAGS) -o $@

moc_main_form.cpp: main_form.h
	$(MOC) $< -o $@

# --------------------------------------------------------------

install:
	install -m 755 lms_session_mgr $(DESTDIR)$(PREFIX)/bin
clean:
	rm -f *.o moc_*.cpp lms_session_mgr

