# $FreeBSD$
#
# Makefile for building the test generation tool

CC=    c++
CFLAGS=-I/usr/local/include -std=c++11
LIBS=  -L/usr/local/lib \
       -lboost_system \
       -lboost_filesystem \
       -lboost_thread \
       -lboost_chrono
OBJS=  utils.o \
       read_annotations.o \
       generate_license.o \
       add_testcase.o \
       generate_test.o

generate_tests: $(OBJS)
	$(CC) $(CFLAGS) -o $(.TARGET) $(.ALLSRC) $(LIBS)

$(OBJS): $(.PREFIX).cpp $(.PREFIX).h
	$(CC) $(CFLAGS) -c $(.PREFIX).cpp

.PHONY: clean \
	fetch_groff \
	fetch_utils \
	run

clean:
	rm -f $(OBJS) generate_tests scripts/utils_list

fetch_groff:
	sh scripts/fetch_groff.sh

fetch_utils:
	sh scripts/fetch_utils.sh

run:
	@echo Generating annotations...
	sh scripts/generate_annot.sh
	@echo Generating test files...
	./generate_tests
