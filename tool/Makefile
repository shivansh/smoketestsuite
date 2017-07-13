# $FreeBSD$
#
# Makefile for building automation tool

CC 			= g++
CFLAGS 	= -I. -std=c++11
OBJS 		= utils.o generate_test.o

generate_test 	: $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC)

$(OBJS) 				: $(.PREFIX).cpp $(.PREFIX).h
	$(CC) $(CFLAGS) -c $(.PREFIX).cpp

.PHONY: clean run

clean:
	rm -rf generate_test generated_tests/* $(OBJS)

run:
	./generate_test
