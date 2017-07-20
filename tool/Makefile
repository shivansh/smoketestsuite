# $FreeBSD$
#
# Makefile for building automation tool

CC 	= g++
CFLAGS 	= -I. -std=c++11
OBJS 	= utils.o add_testcase.o generate_test.o run_tests.o

generate_test 	: $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC)

$(OBJS) 	: $(.PREFIX).cpp $(.PREFIX).h
	$(CC) $(CFLAGS) -c $(.PREFIX).cpp

.PHONY: clean clean_tests run

clean:
	rm -f generate_test $(OBJS)

clean_tests:
	rm -f generated_tests/*

run:
	./generate_test
