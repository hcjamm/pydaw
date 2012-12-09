
#!/usr/bin/make -f

CC  ?= gcc
CXX ?= g++
MOC ?= moc

PREFIX ?= /usr/local

BASE_FLAGS     = -ffast-math -fPIC -msse -msse2 -msse3 -mfpmath=sse -Wall -Isrc -I.
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
BUILD_CXXFLAGS = $(BASE_FLAGS) $(shell pkg-config --cflags liblo QtCore QtGui x11 sm sndfile) $(CXXFLAGS)
LINK_CFLAGS    = -shared -lm $(LDFLAGS) $(shell pkg-config --libs liblo alsa sndfile samplerate)
LINK_CXXFLAGS  = -lm -pthread $(shell pkg-config --libs liblo QtCore QtGui x11 sm sndfile) $(LDFLAGS)

C_OBJS   = src/synth.o
CXX_OBJS = src/synth_qt_gui.o src/moc_synth_qt_gui.o

# --------------------------------------------------------------

all: way_v.so WAY_V_qt

way_v.so: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

WAY_V_qt: $(CXX_OBJS)
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
	install -d $(DESTDIR)$(PREFIX)/lib/dssi/way_v
	install -m 644 way_v.so $(DESTDIR)$(PREFIX)/lib/dssi
	install -m 755 WAY_V_qt $(DESTDIR)$(PREFIX)/lib/dssi/way_v

clean:
	rm -f src/*.o src/moc_*.cpp *.so WAY_V_qt
