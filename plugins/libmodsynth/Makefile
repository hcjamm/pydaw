#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc

PREFIX ?= /usr/local

BASE_FLAGS     =  -mstackrealign -fsingle-precision-constant -fPIC -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -Wall -Isrc
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
BUILD_CXXFLAGS = $(BASE_FLAGS) $(shell pkg-config --cflags liblo QtCore QtGui x11 sm) $(CXXFLAGS)
LINK_CFLAGS    = -shared -lm $(LDFLAGS) $(shell pkg-config --libs liblo alsa)
LINK_CXXFLAGS  = -lm $(shell pkg-config --libs liblo QtCore QtGui x11 sm) $(LDFLAGS)

C_OBJS   = 
CXX_OBJS = 

# --------------------------------------------------------------

all: libmodsynth.so

libmodsynth.so: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

.cpp.o:
	$(CXX) -c $< $(BUILD_CXXFLAGS) -o $@

# --------------------------------------------------------------

install:

clean:
	rm -f *.o
