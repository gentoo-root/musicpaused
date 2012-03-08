.PHONY: first all clean install

SOURCES = main.c dbus.c
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
DEPS = $(patsubst %.c,%.dep,$(SOURCES))

SHELL = sh
CC = gcc
LD = gcc
INSTALL = install -v
RM = rm -f

CFLAGS = -O3 -Wall -fomit-frame-pointer `pkg-config --cflags dbus-1`
LDFLAGS =
LIBS = `pkg-config --libs dbus-1`

first: all

-include $(DEPS)

all:	musicpaused

musicpaused:	$(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

%.dep:  %.c Makefile
	$(SHELL) -ec "$(CC) -x c $(CFLAGS) -MM $< | sed -re 's|([^:]+.o)( *:+)|$(<:.c=.o) $@\2|;'" > $@

%.o:	%.c %.dep Makefile
	$(CC) -x c $(CFLAGS) -c $< -o $@

clean:
	$(RM) musicpaused *.o *.dep

install:
	$(INSTALL) musicpaused /usr/bin

uninstall:
	$(RM) /usr/bin/musicpaused
