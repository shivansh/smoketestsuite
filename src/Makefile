# $FreeBSD$
#
# Makefile for building the test generation tool

CC=    c++
CFLAGS=-I/usr/local/include -std=c++11 -DDEBUG
LIBS=  -L/usr/local/lib \
       -lboost_system \
       -lboost_filesystem
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
	rm -rf $(OBJS) \
		generate_tests \
		scripts/utils_list \
		groff \
		failed_groff \
		generated_tests

fetch_groff:
	sh scripts/fetch_groff.sh

fetch_utils:
	sh scripts/fetch_utils.sh

run:
	@echo Generating annotations...
	sh scripts/generate_annot.sh
	@echo Generating test files...
	./generate_tests
