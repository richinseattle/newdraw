#
# Copyright (C) 2002  Pekka Enberg <Pekka.Enberg@cs.helsinki.fi>
#
# Distributed under the terms of the GNU General Public License
# version 2 or later.
#

LIBS	= -lncurses -lform
CC      = gcc
CFLAGS  = -Wall -g -O2

NEW_DRAW = newdraw

OBJS = \
	ansi-esc.o \
	bin-file.o \
	edit-buffer.o \
	colors.o \
	error.o \
	newdraw.o \
	screen.o

%.o: %.S
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

all: newdraw

newdraw: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NEW_DRAW) $(LIBS)
	mv newdraw .. && mkdir -p ../art
clean:
	rm -f $(NEW_DRAW) core *.o
