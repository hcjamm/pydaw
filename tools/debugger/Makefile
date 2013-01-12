#!/usr/bin/make -f

CC  ?= gcc

BASE_FLAGS     = -O0 -gdwarf-3 -mstackrealign -fsingle-precision-constant -fPIC -mtune=generic -msse -msse2 -msse3 -mfpmath=sse
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
LINK_CFLAGS    = -ldl -lm $(shell pkg-config --libs liblo QtCore QtGui x11 sm alsa sndfile) $(LDFLAGS)

C_OBJS   = main.o

# --------------------------------------------------------------

all: debugger

profile:
	make CFLAGS+=" -pg " LDFLAGS+=" -pg "

debugger: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

clean:
	rm -f *.o *.so debugger

