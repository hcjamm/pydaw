
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

all: lms_modulex.so LMS_MODULEX_qt

lms_modulex.so: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

LMS_MODULEX_qt: $(CXX_OBJS)
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
	install -d $(DESTDIR)$(PREFIX)/lib/dssi/lms_modulex
	install -m 644 lms_modulex.so $(DESTDIR)$(PREFIX)/lib/dssi
	install -m 755 LMS_MODULEX_qt $(DESTDIR)$(PREFIX)/lib/dssi/lms_modulex

clean:
	rm -f src/*.o src/moc_*.cpp *.so LMS_MODULEX_qt
