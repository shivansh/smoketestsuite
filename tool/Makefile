# $FreeBSD$
#
# Makefile for building automation tool

CC 	= g++
CFLAGS 	= -I. -std=c++11
OBJS 	= utils.o read_annotations.o add_testcase.o generate_test.o

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
	@echo Generating annotations...
	./generate_annot.sh
	@echo Generating test files...
	./generate_test
