#!/usr/bin/make -f

CC  ?= gcc

BASE_FLAGS     = -mstackrealign -fsingle-precision-constant -mtune=generic -msse -msse2 -msse3 -mfpmath=sse -fstrength-reduce -fstrict-aliasing -fprefetch-loop-arrays -finline-functions
BUILD_CFLAGS   = $(BASE_FLAGS) $(CFLAGS)
LINK_CFLAGS    = -ldl -lm $(shell pkg-config --libs liblo sndfile) $(LDFLAGS)

C_OBJS   = main.o

# --------------------------------------------------------------

all: debugger

debug:
	make CFLAGS+=" -O0 -gdwarf-3 "

pydaw_benchmark:
	make CFLAGS+=" -O3 "

gprof:
	make CFLAGS+=" -O3 -pg " LDFLAGS+=" -pg "

debugger: $(C_OBJS)
	$(CC) $(C_OBJS) $(LINK_CFLAGS) -o $@

# --------------------------------------------------------------

.c.o:
	$(CC) -c $< $(BUILD_CFLAGS) -o $@

# --------------------------------------------------------------

clean:
	rm -f *.o *.so debugger *.pymap *.wav profile.txt gmon.out

