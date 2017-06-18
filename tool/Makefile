# $FreeBSD$
#
# Makefile for building automation tool

CC 	= g++
CFLAGS 	= -I. -std=c++11
OBJS 	= tool.o

tool 	: $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC)

$(OBJS) : $(.PREFIX).cpp $(.PREFIX).hpp
	$(CC) $(CFLAGS) -c $(.PREFIX).cpp

.PHONY: clean run

clean:
	rm -f tool $(OBJS)

run:
	./tool
