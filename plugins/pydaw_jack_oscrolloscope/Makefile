PREFIX ?=	/usr

CFLAGS =	-O3 $(shell sdl-config --cflags) $(shell pkg-config --cflags jack) -W -std=gnu99
LIBS =		$(shell sdl-config --libs) $(shell pkg-config --libs jack) -lGL -lX11 -lm

OBJS =		main.o video.o audio.o waves.o
BIN =		pydaw_jack_oscrolloscope


all:		$(BIN)

$(BIN): $(OBJS)
		gcc -o $(BIN) $(OBJS) $(LIBS)

%.o: %.c
		gcc -c -o $@ $(CFLAGS) $*.c

clean:
		rm -f $(BIN) $(OBJS) .dep

install:
	install -m 755 $(BIN) $(DESTDIR)$(PREFIX)/lib/dssi/pydaw

dep:
	rm -f .dep; \
	for f in *.c; \
	do \
	    gcc -MM $$f $(CFLAGS) >> .dep || exit 1; \
	done

ifeq (.dep,$(wildcard .dep))
include .dep
endif

