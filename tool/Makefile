# $FreeBSD$
#
# Makefile for building automation tool

CC 	= gcc
CFLAGS 	= -I.
OBJS 	= tool.o

tool 	: $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC)

$(OBJS) : $(.PREFIX).c $(.PREFIX).h
	$(CC) $(CFLAGS) -c $(.PREFIX).c

.PHONY: clean run

clean:
	rm -f tool $(OBJS)

run:
	./tool
