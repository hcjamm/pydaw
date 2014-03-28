#!/usr/bin/make -f

PREFIX ?= /usr
DESTDIR ?= 
CC  ?= gcc

BASE_FLAGS     = -Wall -mstackrealign -fsingle-precision-constant -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fstrength-reduce -fstrict-aliasing -fprefetch-loop-arrays -finline-functions
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
LINK_CFLAGS    = -ldl -lm $(shell pkg-config --libs liblo sndfile) $(LDFLAGS)

C_OBJS   = main.o

# --------------------------------------------------------------

all: pydaw4_render

debug:
	make CFLAGS+=" -O0 -gdwarf-3 -Wstack-usage=4096 -fstack-usage "

pydaw_benchmark:
	make CFLAGS+=" -O3 "

gprof:
	make CFLAGS+=" -O3 -pg " LDFLAGS+=" -pg "

pydaw4_render: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

clean:
	rm -f *.o *.so pydaw*render *.pymap *.wav profile.txt gmon.out

install:
	install -d $(DESTDIR)/$(PREFIX)
	install -d $(DESTDIR)/$(PREFIX)/bin
	install -m 755 pydaw4_render $(DESTDIR)/$(PREFIX)/bin

